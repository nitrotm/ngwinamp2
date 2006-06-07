// ngwinampserver.cpp
#include "../global.h"
#include "../util.h"
#include "../config.h"
#include "../fs.h"
#include "../net.h"
#include "../netaddr.h"
#include "../netdata.h"
#include "../netsnapshot.h"
#include "../netauth.h"
#include "plugin.h"
#include "ngwinamp.h"
#include "ngwinampserver.h"
#include "ngwinampcon.h"
#include "ngwinampuser.h"


int WINAPI NGWINAMPSERVER_thread(NGWINAMPSERVER *pserver);


NGWINAMPSERVER::NGWINAMPSERVER(HWND hwndplugin) : NGWINAMP(hwndplugin), hthread(NULL), swait(INVALID_SOCKET), shares(NULL, "", vector<string>()) {
	this->hrunning = CreateEvent(NULL, TRUE, FALSE, NULL);
	this->hquit = CreateEvent(NULL, TRUE, FALSE, NULL);
}

NGWINAMPSERVER::~NGWINAMPSERVER() {
	this->stop();
	CloseHandle(this->hrunning);
	CloseHandle(this->hquit);
}


bool NGWINAMPSERVER::isrunning(void) {
	NGLOCKER locker(this);

	return (WaitForSingleObject(this->hrunning, 10) == WAIT_OBJECT_0);
}

bool NGWINAMPSERVER::isquit(void) {
	NGLOCKER locker(this);

	return (WaitForSingleObject(this->hquit, 10) == WAIT_OBJECT_0);
}


bool NGWINAMPSERVER::start(void) {
	NGLOCKER locker(this);

	if (this->hthread == NULL && !this->isrunning()) {
		DWORD tid;

		// begin mainloop
		this->hthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)NGWINAMPSERVER_thread, (LPVOID)this, 0, &tid);
		return (this->hthread != NULL && tid != 0);
	}
	return false;
}

bool NGWINAMPSERVER::stop(void) {
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
	this->cfg.read(filename);

	CFGNode server = this->cfg.get("server");

	this->cfg_enabled = server.getchild("enabled").getbool();


	CFGNode network = server.getchild("network");

	this->cfg_address = network.getchild("address").getstr();
	this->cfg_port = (word)network.getchild("port").getuint();
	this->cfg_connection = network.getchild("connection").getuint();
	this->cfg_timeout = network.getchild("timeout").getuint();
	this->cfg_buffersize = network.getchild("buffersize").getuint();
	this->cfg_allowzzip = network.getchild("allowzzip").getbool();


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
		}
	}


	CFGNode sharedefs = server.getchild("shares");

	if (sharedefs.exists("refresh")) {
		this->cfg_sharerefresh = sharedefs.getchild("refresh").getfloat();
	} else {
		this->cfg_sharerefresh = 300.0;
	}
	for (dword i = 0; i < sharedefs.size(); i++) {
		this->appendshare(&this->shares, sharedefs.getchild(i));
	}
}

void NGWINAMPSERVER::appendshare(FSRoot *parent, const CFGNode &share) {
	if (share.isgroup() && !parent->exists(share.getname())) {
		vector<string>	usernames;
		FSRoot			*proot;
		FSNode			*pnode;

		if (share.getchild("users").getstr().length() > 0) {
			usernames = strsplit(share.getchild("users").getstr(), " ", 0, false);
		}
		if (share.exists("localpath") || share.exists("remotepath")) {
			vector<string> exts;

			if (share.getchild("filter").getstr().length() > 0) {
				exts = strsplit(share.getchild("filter").getstr(), " ", 0, false);
			}
			if (share.exists("localpath")) {
				if (pathexists(share.getchild("localpath").getstr())) {
					proot = new FSRoot(parent, share.getname(), usernames, share.getchild("localpath").getstr(), exts, share.getchild("refresh").getfloat(), share.getchild("recursive").getbool());
					parent->add(proot);

					DEBUGWRITE("Local share " + share.getname() + " at " + proot->getfullname());

					for (dword i = 0; i < share.size(); i++) {
						this->appendshare(proot, share.getchild(i));
					}
				}
			} else if (share.exists("remotepath")) {
				NETRESOURCE nr;
				DWORD		success;
				char		path[MAX_PATH];

				memset(&nr, 0, sizeof(NETRESOURCE));
				nr.dwType = RESOURCETYPE_DISK;
				lstrcpyn(path, share.getchild("remotepath").getstr().c_str(), MAX_PATH);
				nr.lpRemoteName = path;
				if (share.exists("remotelogin")) {
					if (share.exists("remotepassword")) {
						success = WNetAddConnection2(&nr, share.getchild("remotepassword").getstr().c_str(), share.getchild("remotelogin").getstr().c_str(), 0);
					} else {
						success = WNetAddConnection2(&nr, NULL, share.getchild("remotelogin").getstr().c_str(), 0);
					}
				} else {
					if (share.exists("remotepassword")) {
						success = WNetAddConnection2(&nr, share.getchild("remotepassword").getstr().c_str(), NULL, 0);
					} else {
						success = WNetAddConnection2(&nr, NULL, NULL, 0);
					}
				}
				if (success == NO_ERROR) {
					DEBUGWRITE("Logon successful to remote share " + share.getname() + " at " + share.getchild("remotepath").getstr());
				} else {
					DEBUGWRITE("Logon failed to remote share " + share.getname() + " at " + share.getchild("remotepath").getstr());
				}
				if (pathexists(share.getchild("remotepath").getstr())) {
					proot = new FSRoot(parent, share.getname(), usernames, share.getchild("remotepath").getstr(), exts, share.getchild("refresh").getfloat(), share.getchild("recursive").getbool());
					parent->add(proot);

					DEBUGWRITE("Remote share " + share.getname() + " at " + proot->getfullname());

					for (dword i = 0; i < share.size(); i++) {
						this->appendshare(proot, share.getchild(i));
					}
				}
			}
		} else if (share.exists("url")) {
			if (pathisurl(share.getchild("url").getstr())) {
				pnode = new FSRoot(parent, share.getname(), usernames, share.getchild("url").getstr());
				parent->add(pnode);

				DEBUGWRITE("Internet share " + share.getname() + " at " + pnode->getfullname());
			}
		} else {
			proot = new FSRoot(parent, share.getname(), usernames);
			parent->add(proot);

			DEBUGWRITE("Node " + share.getname() + " at " + proot->getfullname());

			for (dword i = 0; i < share.size(); i++) {
				this->appendshare(proot, share.getchild(i));
			}
		}
	}
}

void NGWINAMPSERVER::savecfg(const string &filename) {
}


bool NGWINAMPSERVER::init(void) {
	NGLOCKER locker(this);

	// init data
	ResetEvent(this->hquit);
	SetEvent(this->hrunning);
	this->readcfg("c:\\ngwinamp.cfg");
	this->shares.refresh(true);
	this->sharetimer.start();

	// resolve bind address
	SOCKADDR_IN	addr;

	memset(&addr, 0, sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	if (this->cfg_address.length() > 0) {
		PHOSTENT phost = gethostbyname(this->cfg_address.c_str());

		if (phost == NULL) {
			DEBUGWRITE("NGWINAMPSERVER::init() cannot resolve bind address !");
			return false;
		}
		if (phost->h_length != 4) {
			DEBUGWRITE("NGWINAMPSERVER::init() invalid bind address (not IPv4) !");
			return false;
		}
		addr.sin_addr.S_un.S_un_b.s_b1 = phost->h_addr_list[0][0];
		addr.sin_addr.S_un.S_un_b.s_b2 = phost->h_addr_list[0][1];
		addr.sin_addr.S_un.S_un_b.s_b3 = phost->h_addr_list[0][2];
		addr.sin_addr.S_un.S_un_b.s_b4 = phost->h_addr_list[0][3];
	}
	addr.sin_port = htons(this->cfg_port);

	// create listening socket
	dword enabled = 1;

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

void NGWINAMPSERVER::free(void) {
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
	this->shares.clear();
	ResetEvent(this->hquit);
	ResetEvent(this->hrunning);
	this->hthread = NULL;

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

	// refresh shares
	if (this->cfg_sharerefresh > 0.0 && this->sharetimer.pick() > this->cfg_sharerefresh) {
		this->shares.refresh(false);
		this->sharetimer.start();
	}
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

const FSNode* NGWINAMPSERVER::findshare(const string &path) {
	NGLOCKER locker(this);

	return this->shares.find(path);
}

vector<string> NGWINAMPSERVER::getfilepaths(const string &username, const string &path) {
	const FSNode *pnode = this->shares.find(path);

	if (pnode != NULL) {
		if (pnode->isfile()) {
			vector<string> items;

			items.push_back(pnode->getpath());
			return items;
		} else {
			vector<string> items = this->getfilepaths(username, pnode, pnode->listdirectories(username));
			vector<string> files = pnode->listfiles(username);

			for (dword i = 0; i < files.size(); i++) {
				const FSNode *pchild = pnode->get(files[i]);

				if (pnode != NULL) {
					items.push_back(pchild->getpath());
				}
			}
			return items;
		}
	}
	return vector<string>();
}
vector<string> NGWINAMPSERVER::getfilepaths(const string &username, const FSNode *pnode, const vector<string> childs) {
	vector<string> items;

	for (dword i = 0; i < childs.size(); i++) {
		const FSNode *pchild = pnode->get(childs[i]);

		if (pchild != NULL) {
			vector<string> subitems = this->getfilepaths(username, pchild, pchild->listdirectories(username));
			vector<string> files = pchild->listfiles(username);

			for (dword i = 0; i < subitems.size(); i++) {
				items.push_back(subitems[i]);
			}
			for (dword i = 0; i < files.size(); i++) {
				const FSNode *psubchild = pchild->get(files[i]);

				if (psubchild != NULL) {
					items.push_back(psubchild->getpath());
				}
			}
		}
	}
	return items;
}



NGWINAMPUSER* NGWINAMPSERVER::finduser(const string &username) {
	NGLOCKER locker(this);

	for (dword i = 0; i < this->users.size(); i++) {
		if (this->users[i]->username.compare(username) == 0) {
			return this->users[i];
		}
	}
	return NULL;
}


bool NGWINAMPSERVER::authenticate(NGWINAMPCON *pconnection, NETDATA *prequest) {
	NGLOCKER	 locker(this);
	NGWINAMPUSER *puser;
	string		 username, password;
	dword		 code = NGWINAMP_AUTH_NOTDONE;

#ifdef _DEBUG
	char tmp[512];
	sprintf(tmp, "NGWINAMPSERVER::main() request (code=%u,param1=%08X,param2=%08X,flags=%08X,param3=%.02f,size=%08X,size2=%08X)", prequest->hdr.code, prequest->hdr.param1, prequest->hdr.param2, prequest->hdr.flags, prequest->hdr.param3, prequest->hdr.size, prequest->hdr.size2);
	DEBUGWRITE(tmp);
#endif

	// check authentication
	if (prequest->hdr.code == NGWINAMP_REQ_AUTH) { // note: obsolete
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
		puser = this->finduser(username);
		if (puser != NULL) {
			code = puser->authenticate(password, pconnection->address);
			if (code == NGWINAMP_AUTH_SUCCESS) {
				// setup connection options
				if (this->cfg_allowzzip && (prequest->hdr.flags & NGWINAMP_FILTER_ALLOWZZIP) != 0) {
					pconnection->setflags(NGWINAMP_FILTER_ALLOWZZIP);
				}
				pconnection->settimeout(puser->gettimeout());

				// reply
				if (prequest->hdr.code == NGWINAMP_REQ_AUTH) { // note: obsolete
					NETAUTH infos;

					memset(&infos, 0, sizeof(NETAUTH));
					infos.periodecount = 0;
					infos.periodetime = 60;
					infos.timeout = puser->gettimeout();
					pconnection->answer(new NETDATA(NGWINAMP_ANS_AUTH, NGWINAMP_AUTH_SUCCESS, 0, 0, 0.0, &infos, sizeof(NETAUTH)));
				}
				if (prequest->hdr.code == NGWINAMP_REQ_AUTH_EX) {
					NETAUTHEX infos;

					memset(&infos, 0, sizeof(NETAUTHEX));
					infos.vmajor = NGWINAMP_VERSION_MAJOR;
					infos.vminor = NGWINAMP_VERSION_MINOR;
					infos.access = puser->getaccess();
					infos.maxconnection = puser->getmaxcon();
					infos.timeout = puser->gettimeout();
					pconnection->answer(new NETDATA(NGWINAMP_ANS_AUTH_EX, NGWINAMP_AUTH_SUCCESS, 0, 0, 0.0, &infos, sizeof(NETAUTHEX)));
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
	} else { // note: obsolete
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
