// ngwinampcon.cpp
#include "../global.h"
#include "../util.h"
#include "../net.h"
#include "../netaddr.h"
#include "../netdata.h"
#include "../netsnapshot.h"
#include "plugin.h"
#include "ngwinampcon.h"


NGWINAMPCON::NGWINAMPCON(SOCKET s, const SOCKADDR_IN &address, double timeout) : NGLOCK(), s(s), timeout(timeout), defaultflags(0), eof(false) {
	dword enabled = 1;

	ioctlsocket(s, FIONBIO, &enabled);
	memcpy(&this->address, &address, sizeof(SOCKADDR_IN));
	memset(&this->snapshot, 0, sizeof(NETSNAPSHOT));
	this->snapshot.mflags = NGWINAMP_SNAPSHOT_ALL;
	this->timer.start();
	DEBUGWRITE("NGWINAMPCON::NGWINAMPCON() new connection...");
}

NGWINAMPCON::~NGWINAMPCON() {
	this->close();
	DEBUGWRITE("NGWINAMPCON::~NGWINAMPCON() connection destroyed...");
}


void NGWINAMPCON::settimeout(double timeout) {
	NGLOCKER locker(this);

	this->timeout = timeout;
}

void NGWINAMPCON::setflags(dword flags) {
	NGLOCKER locker(this);

	this->defaultflags = flags;
}


bool NGWINAMPCON::isclosed(void) {
	NGLOCKER locker(this);

	return (this->s == INVALID_SOCKET);
}

void NGWINAMPCON::close(void) {
	NGLOCKER locker(this);

	if (this->s != INVALID_SOCKET) {
		closesocket(this->s);
		this->s = INVALID_SOCKET;
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
}

void NGWINAMPCON::shutdown(void) {
	NGLOCKER locker(this);

	this->eof = true;
}


bool NGWINAMPCON::main(void) {
	if (this->recvmsg()) {
		return this->sendmsg();
	}
	return this->sendmsg();
}

bool NGWINAMPCON::recvmsg(void) {
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
	FD_SET(this->s, &sr);
	if (select(0, &sr, &sw, &se, &t) > 0) {
		byte	 buffer[NGWINAMP_NETBUFFERSIZE];
		int		 len = recv(this->s, (char*)buffer, sizeof(buffer), 0);

		if (len > 0) {
			if (!this->eof) {
				this->recvbuffer.append(buffer, len);
			}
		} else {
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				this->close();
				return false;
			}
		}
	}
	if (this->timeout > 0.0 && this->timer.pick() > this->timeout) {
		this->close();
		return false;
	}
	return true;
}

bool NGWINAMPCON::sendmsg(void) {
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
	FD_SET(this->s, &sw);
	if (this->sendbuffer.size() > 0 && select(0, &sr, &sw, &se, &t) > 0) {
		byte	buffer[NGWINAMP_NETBUFFERSIZE];
		int		len;

		len = this->sendbuffer.read(buffer, 0, NGWINAMP_NETBUFFERSIZE);
		len = send(this->s, (const char*)buffer, len, 0);
		if (len > 0) {
			this->timer.start();
			this->sendbuffer.erase(0, len);
		}
	} else {
		if (this->eof) {
			this->close();
			return false;
		}
	}
	if (this->timeout > 0.0 && this->timer.pick() > this->timeout) {
		this->close();
		return false;
	}
	return true;
}


NETDATA* NGWINAMPCON::request(void) {
	NGLOCKER locker(this);

	if (!this->isclosed() && this->requests.size() > 0) {
		NETDATA *prequest = this->requests[0];

		this->requests.erase(this->requests.begin());

		prequest->uncompress();
		return prequest;
	}
	return NULL;
}

bool NGWINAMPCON::answer(NETDATA *panswer) {
	NGLOCKER locker(this);

	if (!this->isclosed() && !this->eof) {
		panswer->hdr.flags |= this->defaultflags;
		panswer->compress();

#ifdef _DEBUG
		char tmp[512];
		sprintf(tmp, "NGWINAMPCON::enqueue() answer (code=%u,param1=%08X,param2=%08X,flags=%08X,param3=%.02f,size=%08X,size2=%08X)", panswer->hdr.code, panswer->hdr.param1, panswer->hdr.param2, panswer->hdr.flags, panswer->hdr.param3, panswer->hdr.size, panswer->hdr.size2);
		DEBUGWRITE(tmp);
#endif

		this->answers.push_back(panswer);
		return true;
	}
	delete panswer;
	return false;
}
