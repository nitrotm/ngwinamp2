// ngwinampserver.cpp
#include "plugin.h"


NGWINAMPSERVER::NGWINAMPSERVER(HWND hwndplugin) : NGWINAMP(hwndplugin), hthread(NULL), swait(INVALID_SOCKET) {
	this->hrunning = CreateEvent(NULL, TRUE, FALSE, NULL);
	this->hquit = CreateEvent(NULL, TRUE, FALSE, NULL);
}

NGWINAMPSERVER::~NGWINAMPSERVER() {
	this->stop();
	CloseHandle(this->hrunning);
	CloseHandle(this->hquit);
}


bool NGWINAMPSERVER::isrunning() {
	NGLOCKER locker(this);

	return (WaitForSingleObject(this->hrunning, 10) == WAIT_OBJECT_0);
}

bool NGWINAMPSERVER::isquit() {
	NGLOCKER locker(this);

	return (WaitForSingleObject(this->hquit, 10) == WAIT_OBJECT_0);
}


bool NGWINAMPSERVER::start() {
	NGLOCKER locker(this);

	if (this->hthread == NULL && !this->isrunning()) {
		DWORD tid;

		// begin mainloop
		this->hthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)NGWINAMPSERVER_thread, (LPVOID)this, 0, &tid);
		return (this->hthread != NULL && tid != 0);
	}
	return false;
}

bool NGWINAMPSERVER::stop() {
	// request mainloop end
	this->lock();
	SetEvent(this->hquit);
	this->unlock();

	// wait for mainloop end
	while (this->isrunning()) {
		Sleep(5);
	}

	// free all data
	this->lock();
	ResetEvent(this->hquit);
	this->hthread = NULL;
	this->unlock();
	return true;
}


void NGWINAMPSERVER::readcfg(const string &filename) {
	this->cfg.read("c:\\test.cfg");

	CFGNode server = this->cfg.get("server");

	this->cfg_enabled = server.getchild("enabled").getbool();


	CFGNode network = server.getchild("network");

	this->cfg_address = network.getchild("address").getstr();
	this->cfg_port = (word)network.getchild("port").getuint();
	this->cfg_connection = network.getchild("connection").getuint();
	this->cfg_timeout = network.getchild("timeout").getuint();
	this->cfg_buffersize = network.getchild("buffersize").getuint();


	CFGNode users = server.getchild("users");

	for (dword i = 0; i < users.size(); i++) {
		CFGNode user = users.getchild(i);

		if (user.getchild("enabled").getbool()) {
			NGWINAMPUSER    *puser = new NGWINAMPUSER(this, user.getname(), user.getchild("password").getstr());
			vector<NETADDR>	allow = NETADDR::parse(user.getchild("allow").getstr());
			vector<NETADDR>	deny = NETADDR::parse(user.getchild("deny").getstr());

			for (dword i = 0; i < allow.size(); i++) {
				puser->allow(allow[i]);
			}
			for (dword i = 0; i < deny.size(); i++) {
				puser->deny(deny[i]);
			}
			puser->setpolicies(puser->parsepolicies(user.getchild("access")), user.getchild("connection").getuint(), user.getchild("timeout").getfloat());
			this->users.push_back(puser);

//			DEBUGWRITE(user.getname().c_str());
		}
	}


	CFGNode shares = server.getchild("shares");
	vector<string> exts;

	for (dword i = 0; i < shares.size(); i++) {
//		DEBUGWRITE(shares.getchild(i).getname().c_str());
	}
	exts.push_back(".zip");
	exts.push_back(".txt");
	this->shares.add(FSNode("C:\\Downloads", exts, true));
}

void NGWINAMPSERVER::savecfg(const string &filename) {
}


bool NGWINAMPSERVER::init(void) {
	NGLOCKER locker(this);

	// init data
	ResetEvent(this->hquit);
	SetEvent(this->hrunning);
	this->readcfg("c:\\test.cfg");

	// create listening socket
	SOCKADDR_IN	addr;
	dword		enabled = 1;

	memset(&addr, 0, sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(this->cfg_port);

	this->swait = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->swait == INVALID_SOCKET) {
		DEBUGWRITE("NGWINAMPSERVER::init() cannot create socket !");
		return false;
	}
	if (bind(this->swait, (const struct sockaddr*)&addr, sizeof(SOCKADDR_IN)) != 0) {
		DEBUGWRITE("NGWINAMPSERVER::init() cannot bind !");
		return false;
	}
	if (listen(this->swait, this->cfg_connection) != 0) {
		DEBUGWRITE("NGWINAMPSERVER::init() cannot listen !");
		return false;
	}
	if (ioctlsocket(this->swait, FIONBIO, &enabled) != 0) {
		DEBUGWRITE("NGWINAMPSERVER::init() cannot change ioctl !");
		return false;
	}
	DEBUGWRITE("NGWINAMPSERVER::init()");
	return true;
}

void NGWINAMPSERVER::free() {
	NGLOCKER locker(this);

	// close all connections
	for (dword i = 0; i < this->connections.size(); i++) {
		delete this->connections[i];
	}
	for (dword i = 0; i < this->users.size(); i++) {
		delete this->users[i];
	}
	this->connections.clear();
	this->users.clear();

	// destroy listening socket
	if (this->swait != INVALID_SOCKET) {
		closesocket(this->swait);
		this->swait = INVALID_SOCKET;
	}

	// free data
	ResetEvent(this->hquit);
	ResetEvent(this->hrunning);

	DEBUGWRITE("NGWINAMPSERVER::free()");
}


void NGWINAMPSERVER::main(void) {
	NGLOCKER locker(this);

	// check for incoming connections
	this->accept();

	// check authorized connections
	for (dword i = 0; i < this->users.size(); i++) {
		this->users[i]->main();
	}

	// allow/deny new connections
	for (dword i = 0; i < this->connections.size(); i++) {
		if (this->connections[i]->recvmsg() && this->connections[i]->sendmsg()) {
			NETDATA *prequest = this->connections[i]->request();

			if (prequest != NULL) {
				if (this->authenticate(this->connections[i], prequest)) {
					this->connections.erase(this->connections.begin() + i);
				} else {
					this->connections[i]->shutdown();
				}
				delete prequest;
			}
		}
	}

	// remove closed connections
	this->gc();
}

void NGWINAMPSERVER::accept(void) {
	NGLOCKER locker(this);
	fd_set	 sr, sw, se;
	timeval	 t = { 0, NGWINAMP_NETBLOCKTIME };

	// check for incomming connection
	FD_ZERO(&sr);
	FD_ZERO(&sw);
	FD_ZERO(&se);
	FD_SET(this->swait, &sr);
	if (select(0, &sr, &sw, &se, &t) > 0) {
		SOCKADDR_IN	addr;
		SOCKET		sa;
		int			len = sizeof(SOCKADDR_IN);

		// ouverture de la connexion
		sa = ::accept(this->swait, (sockaddr*)&addr, &len);
		if (sa != INVALID_SOCKET) {
			this->connections.push_back(new NGWINAMPCON(sa, addr, NGWINAMP_NETINITIALTIMEOUT));
		}
	}
}

void NGWINAMPSERVER::gc(void) {
	NGLOCKER locker(this);

	for (dword i = 0; i < this->connections.size(); i++) {
		if (this->connections[i]->isclosed()) {
			delete this->connections[i];
			this->connections.erase(this->connections.begin() + i);
			this->gc();
			break;
		}
	}
}


NGWINAMPUSER* NGWINAMPSERVER::find(const string &username) {
	for (dword i = 0; i < this->users.size(); i++) {
		if (this->users[i]->username.compare(username) == 0) {
			return this->users[i];
		}
	}
	return NULL;
}


bool NGWINAMPSERVER::authenticate(NGWINAMPCON *pconnection, NETDATA *prequest) {
	NGWINAMPUSER *puser;
	string		 username, password;
	dword		 code = NGWINAMP_AUTH_NOTDONE;

	char tmp[512];
	sprintf(tmp, "NGWINAMPSERVER::main() request (code=%u,param1=%08X,param2=%08X,flags=%08X,param3=%.02f,size=%08X,size2=%08X)", prequest->hdr.code, prequest->hdr.param1, prequest->hdr.param2, prequest->hdr.flags, prequest->hdr.param3, prequest->hdr.size, prequest->hdr.size2);
	DEBUGWRITE(tmp);

	// check authentication
	if (prequest->hdr.code == NGWINAMP_REQ_AUTH) {
		if (prequest->hdr.size > 0) {
			// password -> "admin"
			username = "admin";
			password = prequest->buffer.tostring();
		} else {
			// empty -> "guest"
			username = "guest";
		}
	} else if (prequest->hdr.code == NGWINAMP_REQ_AUTH_EX) {
		// read username & password
		word offset, length;

		if (prequest->buffer.read(&length, 0, 2) == 2) {
			offset = 2;
			username = prequest->buffer.tostring(offset, length);
			offset += length;
			if (prequest->buffer.read(&length, offset, 2) == 2) {
				offset += 2;
				password = prequest->buffer.tostring(offset, length);
			}
		}
	}
	if (username.length() > 0) {
		puser = this->find(username);
		if (puser != NULL) {
			code = puser->authenticate(password, pconnection->address);
			if (code == NGWINAMP_AUTH_SUCCESS) {
				// setup connection options
				if ((prequest->hdr.flags & NGWINAMP_FILTER_ALLOWZZIP) != 0) {
					pconnection->setflags(NGWINAMP_FILTER_ALLOWZZIP);
				}
				pconnection->settimeout(puser->gettimeout());

				// reply
				if (prequest->hdr.code == NGWINAMP_REQ_AUTH) {
					NETSERVERINFO infos;

					memset(&infos, 0, sizeof(NETSERVERINFO));
//					infos.periodecount = ;
//					infos.periodetime = ;
					infos.timeout = puser->gettimeout();
					pconnection->answer(new NETDATA(NGWINAMP_ANS_AUTH, NGWINAMP_AUTH_SUCCESS, 0, 0, 0.0, &infos, sizeof(NETSERVERINFO)));
				}
				if (prequest->hdr.code == NGWINAMP_REQ_AUTH_EX) {
					NETSERVERINFOEX infos;

					memset(&infos, 0, sizeof(NETSERVERINFOEX));
//					infos.periodecount = ;
//					infos.periodetime = ;
					infos.timeout = puser->gettimeout();
					infos.access = puser->getaccess();
					pconnection->answer(new NETDATA(NGWINAMP_ANS_AUTH_EX, NGWINAMP_AUTH_SUCCESS, 0, 0, 0.0, &infos, sizeof(NETSERVERINFOEX)));
				}

				// move connection into authorized pool
				puser->add(pconnection);
				return true;
			}
		}
		code = NGWINAMP_AUTH_FAILURE;
	}
	if (prequest->hdr.code == NGWINAMP_REQ_AUTH_EX) {
		pconnection->answer(new NETDATA(NGWINAMP_ANS_AUTH_EX, code, 0, 0, 0.0));
	} else {
		pconnection->answer(new NETDATA(NGWINAMP_ANS_AUTH, code, 0, 0, 0.0));
	}
	return false;
}



int WINAPI NGWINAMPSERVER_thread(NGWINAMPSERVER *pserver) {
	if (pserver->init()) {
		while (!pserver->isquit()) {
			pserver->main();
		}
	}
	pserver->free();
	return 0;
}
