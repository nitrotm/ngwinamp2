// ngwinampclient.cpp
#include "client.h"
#include "../net.h"
#include "../netaddr.h"
#include "../netdata.h"
#include "ngwinampclient.h"
#include "mainwnd.h"


NGWINAMPCLIENT::NGWINAMPCLIENT(NGMainWnd *clientwnd) : NGLOCK(), clientwnd(clientwnd), sclient(INVALID_SOCKET), hthread(NULL), hrunning(NULL), hquit(NULL), defaultflags(0), timeout(0), eof(false) {
	this->hrunning = CreateEvent(NULL, TRUE, FALSE, NULL);
	this->hquit = CreateEvent(NULL, TRUE, FALSE, NULL);
}

NGWINAMPCLIENT::~NGWINAMPCLIENT() {
	this->stop();
	CloseHandle(this->hrunning);
	CloseHandle(this->hquit);
}


bool NGWINAMPCLIENT::isrunning(void) {
	NGLOCKER locker(this);

	return (WaitForSingleObject(this->hrunning, 10) == WAIT_OBJECT_0);
}

bool NGWINAMPCLIENT::isquit(void) {
	NGLOCKER locker(this);

	return (WaitForSingleObject(this->hquit, 10) == WAIT_OBJECT_0);
}


bool NGWINAMPCLIENT::start(const string &address, const word port) {
	NGLOCKER locker(this);

	if (this->hthread == NULL && !this->isrunning()) {
		DWORD tid;

		// begin mainloop
		this->cfg_address = address;
		this->cfg_port = port;
		this->hthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)NGWINAMPCLIENT_thread, (LPVOID)this, 0, &tid);
		return (this->hthread != NULL && tid != 0);
	}
	return false;
}

bool NGWINAMPCLIENT::stop(void) {
	// request mainloop end
	this->shutdown();

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

void NGWINAMPCLIENT::shutdown(void) {
	NGLOCKER locker(this);

	this->eof = true;
}


bool NGWINAMPCLIENT::authenticate(const string &username, const string &password) {
	NGBUFFER data(4 + username.length() + password.length());
	int		 length;
	int		 offset = 0;

	length = username.length();
	data.write(&length, offset, 2);
	offset += 2;

	data.write(username.c_str(), offset, length);
	offset += length;

	length = password.length();
	data.write(&length, offset, 2);
	offset += 2;

	data.write(password.c_str(), offset, length);
	offset += length;

	return this->answer(new NETDATA(NGWINAMP_REQ_AUTH_EX, 0, 0, 0, 0.0, data));
}

bool NGWINAMPCLIENT::requestSnapshot(void) {
	return this->answer(new NETDATA(NGWINAMP_REQ_GETSNAPSHOT, 0, 0, 0, 0.0));
}



bool NGWINAMPCLIENT::init(void) {
	NGLOCKER locker(this);

	// init data
	ResetEvent(this->hquit);
	SetEvent(this->hrunning);
	this->timer.start();
	this->defaultflags = NGWINAMP_FILTER_ALLOWZZIP;
	this->timeout = 0;
	this->eof = false;

	// resolve server address
	SOCKADDR_IN	addr;
	PHOSTENT	phost = gethostbyname(this->cfg_address.c_str());

	if (phost == NULL) {
		DEBUGWRITE("NGWINAMPCLIENT::init() cannot resolve server address !");
		this->clientwnd->onnet_connected(false, "Networking error", "Cannot resolve server address !");
		return false;
	}
	if (phost->h_length != 4) {
		DEBUGWRITE("NGWINAMPCLIENT::init() invalid server address (not IPv4) !");
		this->clientwnd->onnet_connected(false, "Networking error", "Server address not IPv4 !");
		return false;
	}
	memset(&addr, 0, sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_un_b.s_b1 = phost->h_addr_list[0][0];
	addr.sin_addr.S_un.S_un_b.s_b2 = phost->h_addr_list[0][1];
	addr.sin_addr.S_un.S_un_b.s_b3 = phost->h_addr_list[0][2];
	addr.sin_addr.S_un.S_un_b.s_b4 = phost->h_addr_list[0][3];
	addr.sin_port = htons(this->cfg_port);

	// create socket
	dword enabled = 1;

	this->sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->sclient == INVALID_SOCKET) {
		DEBUGWRITE("NGWINAMPCLIENT::init() cannot create socket !");
		this->clientwnd->onnet_connected(false, "Networking error", "Socket failure !");
		return false;
	}
	if (connect(this->sclient, (const struct sockaddr*)&addr, sizeof(SOCKADDR_IN)) != 0) {
		DEBUGWRITE("NGWINAMPCLIENT::init() cannot connect to server !");
		this->clientwnd->onnet_connected(false, "Networking error", "Cannot connect to server,\r\nconnection refused !");
		return false;
	}
	if (ioctlsocket(this->sclient, FIONBIO, &enabled) != 0) {
		DEBUGWRITE("NGWINAMPCLIENT::init() cannot change ioctl !");
		this->clientwnd->onnet_connected(false, "Networking error", "Socket failure !");
		return false;
	}
	DEBUGWRITE("NGWINAMPCLIENT::init()");
	return this->clientwnd->onnet_connected(true, "", "");
}

void NGWINAMPCLIENT::free(void) {
	NGLOCKER locker(this);

	// close connection
	if (this->sclient != INVALID_SOCKET) {
		closesocket(this->sclient);
		this->sclient = INVALID_SOCKET;
	}
	for (dword i = 0; i < this->requests.size(); i++) {
		delete this->requests[i];
	}
	for (dword i = 0; i < this->answers.size(); i++) {
		delete this->answers[i];
	}
	this->requests.clear();
	this->answers.clear();
	this->recvbuffer.free();
	this->sendbuffer.free();

	// free data
	ResetEvent(this->hquit);
	ResetEvent(this->hrunning);
	this->hthread = NULL;

	DEBUGWRITE("NGWINAMPCLIENT::free()");
	this->clientwnd->onnet_disconnect();
}


bool NGWINAMPCLIENT::main(void) {
	NGLOCKER locker(this);

	// check connection
	if (this->recvmsg()) {
		if (!this->process()) {
			this->shutdown();
		}
		return this->sendmsg();
	}
	if (!this->process()) {
		this->shutdown();
	}
	return this->sendmsg();
}

bool NGWINAMPCLIENT::recvmsg(void) {
	NGLOCKER locker(this);
	fd_set	 sr, sw, se;
	timeval	 t = { 0, NGWINAMP_NETBLOCKTIME };

	// parse incoming request
	if (this->recvbuffer.size() >= sizeof(NETHDR)) {
		NETHDR hdr;

		this->recvbuffer.read(&hdr, 0, sizeof(NETHDR));
		if (this->recvbuffer.size() >= sizeof(NETHDR) + hdr.size) {
			this->requests.push_back(new NETDATA(hdr, this->recvbuffer.get(sizeof(NETHDR), hdr.size)));
			this->recvbuffer.erase(0, sizeof(NETHDR) + hdr.size);
		}
	}

	// check pending data (requests)
	FD_ZERO(&sr);
	FD_ZERO(&sw);
	FD_ZERO(&se);
	FD_SET(this->sclient, &sr);
	if (select(0, &sr, &sw, &se, &t) > 0) {
		byte	 buffer[NGWINAMP_NETBUFFERSIZE];
		int		 len = recv(this->sclient, (char*)buffer, sizeof(buffer), 0);

		if (len > 0) {
			if (!this->eof) {
				this->recvbuffer.append(buffer, len);
			}
		} else {
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				this->shutdown();
				return false;
			}
		}
	}
	if (this->timeout > 0.0 && this->timer.pick() > this->timeout) {
		this->shutdown();
		return false;
	}
	return true;
}

bool NGWINAMPCLIENT::sendmsg(void) {
	NGLOCKER locker(this);
	fd_set	 sr, sw, se;
	timeval	 t = { 0, NGWINAMP_NETBLOCKTIME };

	// generate outgoing answer
	if (this->sendbuffer.size() == 0 && this->answers.size() > 0) {
		this->sendbuffer.alloc(sizeof(NETHDR) + this->answers[0]->hdr.size);
		this->sendbuffer.write(&this->answers[0]->hdr, 0, sizeof(NETHDR));
		this->sendbuffer.write(this->answers[0]->buffer, sizeof(NETHDR), this->answers[0]->hdr.size);
		delete this->answers[0];
		this->answers.erase(this->answers.begin());
	}

	// send pending data (answers)
	FD_ZERO(&sr);
	FD_ZERO(&sw);
	FD_ZERO(&se);
	FD_SET(this->sclient, &sw);
	if (this->sendbuffer.size() > 0 && select(0, &sr, &sw, &se, &t) > 0) {
		byte	buffer[NGWINAMP_NETBUFFERSIZE];
		int		len;

		len = this->sendbuffer.read(buffer, 0, NGWINAMP_NETBUFFERSIZE);
		len = send(this->sclient, (const char*)buffer, len, 0);
		if (len > 0) {
			this->timer.start();
			this->sendbuffer.erase(0, len);
		}
	} else {
		if (this->eof) {
			return false;
		}
	}
	if (this->timeout > 0.0 && this->timer.pick() > this->timeout) {
		this->shutdown();
		return false;
	}
	return true;
}

bool NGWINAMPCLIENT::answer(NETDATA *panswer) {
	NGLOCKER locker(this);

	if (this->isrunning() && !this->eof) {
		panswer->hdr.flags |= this->defaultflags;
		panswer->compress();

#ifdef _DEBUG
		char tmp[512];
		sprintf(tmp, "NGWINAMPCLIENT::enqueue() answer (code=%u,param1=%08X,param2=%08X,flags=%08X,param3=%.02f,size=%08X,size2=%08X)", panswer->hdr.code, panswer->hdr.param1, panswer->hdr.param2, panswer->hdr.flags, panswer->hdr.param3, panswer->hdr.size, panswer->hdr.size2);
		DEBUGWRITE(tmp);
#endif

		this->answers.push_back(panswer);
		return true;
	}
	delete panswer;
	return false;
}

bool NGWINAMPCLIENT::process(void) {
	NGLOCKER locker(this);

	if (this->isrunning() && this->requests.size() > 0) {
		NETDATA *prequest = this->requests[0];

		this->requests.erase(this->requests.begin());

		prequest->uncompress();

#ifdef _DEBUG
		char tmp[512];
		sprintf(tmp, "NGWINAMPCLIENT::process() process (code=%u,param1=%08X,param2=%08X,flags=%08X,param3=%.02f,size=%08X,size2=%08X)", prequest->hdr.code, prequest->hdr.param1, prequest->hdr.param2, prequest->hdr.flags, prequest->hdr.param3, prequest->hdr.size, prequest->hdr.size2);
		DEBUGWRITE(tmp);
#endif

		switch (prequest->hdr.code) {
		case NGWINAMP_ANS_AUTH_EX:
			NETAUTHEX authex;

			if (prequest->buffer.read(&authex, 0, sizeof(NETAUTHEX)) != sizeof(NETAUTHEX)) {
				return this->clientwnd->onnet_authfailed(prequest->hdr.param1);
			}
			return this->clientwnd->onnet_authsuccess(authex);

		case NGWINAMP_ANS_VOLUME:
			return this->clientwnd->onnet_setvolume(prequest->hdr.param3);
		case NGWINAMP_ANS_PAN:
			return this->clientwnd->onnet_setpan(prequest->hdr.param3);
		case NGWINAMP_ANS_POS:
			return this->clientwnd->onnet_setposition(prequest->hdr.param3, prequest->hdr.param2, prequest->hdr.param1);

		case NGWINAMP_ANS_PLNAMES:
			break;
		case NGWINAMP_ANS_PLFILES:
			break;
		case NGWINAMP_ANS_PLPOS:
			break;
		case NGWINAMP_ANS_PLSHUFFLE:
			return this->clientwnd->onnet_setshuffle(prequest->hdr.param1 == NGWINAMP_ALL);
		case NGWINAMP_ANS_PLREPEAT:
			return this->clientwnd->onnet_setrepeat(prequest->hdr.param1 == NGWINAMP_ALL);

		case NGWINAMP_ANS_BWDIRECTORY:
			break;
		case NGWINAMP_ANS_BWFILES:
			break;

		case NGWINAMP_ANS_SNAPSHOT_EX:
			break;
		}
		return true;
	}
	return true;
}


int WINAPI NGWINAMPCLIENT_thread(NGWINAMPCLIENT *pclient) {
	if (pclient->init()) {
		while (!pclient->isquit()) {
			if (!pclient->main()) {
				break;
			}
		}
	}
	pclient->free();
	return 0;
}
