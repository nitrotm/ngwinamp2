// ngwinampclient.cpp
#include "../global.h"
#include "../util.h"
#include "../net.h"
#include "../netaddr.h"
#include "../netdata.h"
#include "../netauth.h"
#include "../netsnapshot.h"
#include "client.h"
#include "ngwinampclient.h"
#include "mainwnd.h"


int WINAPI NGWINAMPCLIENT_thread(NGWINAMPCLIENT *pclient);


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

bool NGWINAMPCLIENT::hasaccess(const dword access) {
	if ((this->access & access) == access) {
		return true;
	}
	return false;
}

bool NGWINAMPCLIENT::process(void) {
	NGLOCKER locker(this);

	if (this->requests.size() > 0) {
		NETDATA *prequest = this->requests[0];

		this->requests.erase(this->requests.begin());
		locker.release();

		prequest->uncompress();

#ifdef _DEBUG
		char tmp[512];
		sprintf(tmp, "NGWINAMPCLIENT::process() process (code=%u,param1=%08X,param2=%08X,flags=%08X,param3=%.02f,size=%08X,size2=%08X)", prequest->hdr.code, prequest->hdr.param1, prequest->hdr.param2, prequest->hdr.flags, prequest->hdr.param3, prequest->hdr.size, prequest->hdr.size2);
		DEBUGWRITE(tmp);
#endif

		NGREADER reader(prequest->buffer);
		NGWRITER writer;

		switch (prequest->hdr.code) {
		case NGWINAMP_ANS_AUTH_EX:
			NETAUTHEX authex;

			if (prequest->buffer.read(&authex, 0, sizeof(NETAUTHEX)) != sizeof(NETAUTHEX)) {
				return this->clientwnd->onnet_authfailed(prequest->hdr.param1);
			}
			this->access = authex.access;
			return this->clientwnd->onnet_authsuccess(authex);

		case NGWINAMP_ANS_VOLUME:
			return this->clientwnd->onnet_setvolume(prequest->hdr.param3);
		case NGWINAMP_ANS_PAN:
			return this->clientwnd->onnet_setpan(prequest->hdr.param3);
		case NGWINAMP_ANS_POS:
			return this->clientwnd->onnet_setposition(prequest->hdr.param3, prequest->hdr.param2, prequest->hdr.param1);

		case NGWINAMP_ANS_PLNAMES:
			{
				vector<dword>	indexes;
				vector<string>	names;

				for (dword i = 0; i < prequest->hdr.param1; i++) {
					indexes.push_back(reader.readDword());
					names.push_back(reader.readWordString());
					if (reader.isError()) {
						return false;
					}
				}
				return this->clientwnd->onnet_playlistnames(names, indexes, prequest->hdr.param2);
			}
			break;
		case NGWINAMP_ANS_PLFILES:
			{
				vector<dword>	indexes;
				vector<string>	files;

				for (dword i = 0; i < prequest->hdr.param1; i++) {
					indexes.push_back(reader.readDword());
					files.push_back(reader.readWordString());
					if (reader.isError()) {
						return false;
					}
				}
				return this->clientwnd->onnet_playlistfiles(files, indexes, prequest->hdr.param2);
			}
			break;
		case NGWINAMP_ANS_PLPOS:
			return this->clientwnd->onnet_playlistpos(prequest->hdr.param1, prequest->hdr.param2);
		case NGWINAMP_ANS_PLSHUFFLE:
			return this->clientwnd->onnet_setshuffle(prequest->hdr.param1 == NGWINAMP_ALL);
		case NGWINAMP_ANS_PLREPEAT:
			return this->clientwnd->onnet_setrepeat(prequest->hdr.param1 == NGWINAMP_ALL);

		case NGWINAMP_ANS_BWDIRECTORIES:
			{
				string			path = reader.readWordString();
				vector<dword>	types;
				vector<dword>	childrens;
				vector<string>	directories;

				if (reader.isError()) {
					return false;
				}
				for (dword i = 0; i < prequest->hdr.param1; i++) {
					types.push_back(reader.readDword());
					childrens.push_back(reader.readDword());
					directories.push_back(reader.readWordString());
					if (reader.isError()) {
						return false;
					}
				}
				return this->clientwnd->onnet_directories(path, directories, types, childrens);
			}
			break;
		case NGWINAMP_ANS_BWFILES:
			{
				string			path = reader.readWordString();
				vector<dword>	types;
				vector<dword>	sizes;
				vector<string>	files;

				if (reader.isError()) {
					return false;
				}
				for (dword i = 0; i < prequest->hdr.param1; i++) {
					types.push_back(reader.readDword());
					sizes.push_back(reader.readDword());
					files.push_back(reader.readWordString());
					if (reader.isError()) {
						return false;
					}
				}
				return this->clientwnd->onnet_files(path, files, types, sizes);
			}
			break;

		case NGWINAMP_ANS_SNAPSHOT_EX:
			{
				for (dword i = 0; i < prequest->hdr.param1; i++) {
					dword type = reader.readDword();
					dword size = reader.readDword();
					dword offset = 0;

					switch (type) {
					case NGWINAMP_SNAPSHOT_SN_VOLUME:
						{
							double volume = reader.readDouble();

							offset += sizeof(double);
							if (reader.isError()) {
								return false;
							}
							this->clientwnd->onnet_setvolume(volume);
						}
						break;
					case NGWINAMP_SNAPSHOT_SN_PAN:
						{
							double pan = reader.readDouble();

							offset += sizeof(double);
							if (reader.isError()) {
								return false;
							}
							this->clientwnd->onnet_setpan(pan);
						}
						break;
					case (NGWINAMP_SNAPSHOT_SN_POSMS | NGWINAMP_SNAPSHOT_SN_LENGTH):
						{
							dword posms = reader.readDword();
							dword length = reader.readDword();

							offset += 2 * sizeof(dword);
							if (reader.isError()) {
								return false;
							}
							this->clientwnd->onnet_setposition((double)posms / (double)length, posms, length);
						}
						break;
					case (NGWINAMP_SNAPSHOT_PL_POS | NGWINAMP_SNAPSHOT_PL_LENGTH):
						{
							dword pos = reader.readDword();
							dword length = reader.readDword();

							offset += 2 * sizeof(dword);
							if (reader.isError()) {
								return false;
							}
							this->clientwnd->onnet_playlistpos(pos, length);
						}
						break;
					case NGWINAMP_SNAPSHOT_PL_SHUFFLE:
						{
							bool shuffle = reader.readBool();

							offset += sizeof(byte);
							if (reader.isError()) {
								return false;
							}
							this->clientwnd->onnet_setshuffle(shuffle);
						}
						break;
					case NGWINAMP_SNAPSHOT_PL_REPEAT:
						{
							bool repeat = reader.readBool();

							offset += sizeof(byte);
							if (reader.isError()) {
								return false;
							}
							this->clientwnd->onnet_setrepeat(repeat);
						}
						break;
					}
					reader.skip(size - offset);
					if (reader.isError()) {
						return false;
					}
				}
			}
			break;

		case NGWINAMP_ANSA_SHARES:
			break;
		case NGWINAMP_ANSA_USERS:
			break;
		case NGWINAMP_ANSA_CLIENTS:
			break;
		}
		return true;
	}
	return false;
}


bool NGWINAMPCLIENT::authenticate(const string &username, const string &password) {
	NGWRITER writer;

	writer.writeWordString(username);
	writer.writeWordString(password);
	return this->answer(new NETDATA(NGWINAMP_REQ_AUTH_EX, 0, 0, 0, 0.0, writer));
}

bool NGWINAMPCLIENT::request_snapshot(const double timeout) {
	if (timeout <= 0.0) {
		return this->answer(new NETDATA(NGWINAMP_REQ_GETSNAPSHOT, 0, 0, 0, 0.0));
	}
	return this->answer(new NETDATA(NGWINAMP_REQ_GETSNAPSHOT_EX, 0, 0, 0, timeout));
}

bool NGWINAMPCLIENT::request_sn_prev(void) {
	return this->answer(new NETDATA(NGWINAMP_REQ_PREV, 0, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_sn_play(void) {
	return this->answer(new NETDATA(NGWINAMP_REQ_PLAY, 0, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_sn_pause(void) {
	return this->answer(new NETDATA(NGWINAMP_REQ_PAUSE, 0, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_sn_stop(void) {
	return this->answer(new NETDATA(NGWINAMP_REQ_STOP, 0, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_sn_next(void) {
	return this->answer(new NETDATA(NGWINAMP_REQ_NEXT, 0, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_sn_setvolume(const double volume) {
	return this->answer(new NETDATA(NGWINAMP_REQ_SETVOLUME, 0, 0, 0, volume));
}
bool NGWINAMPCLIENT::request_sn_setpan(const double pan) {
	return this->answer(new NETDATA(NGWINAMP_REQ_SETPAN, 0, 0, 0, pan));
}
bool NGWINAMPCLIENT::request_sn_setpos(const double pos) {
	return this->answer(new NETDATA(NGWINAMP_REQ_SETPOS, 0, 0, 0, pos));
}

bool NGWINAMPCLIENT::request_pl_addfiles(const dword index, const vector<string> &paths) {
	NGWRITER writer;

	for (long i = (paths.size() - 1); i >= 0 ; i--) {
		writer.writeDword(index);
		writer.writeWordString(paths[i]);
	}
	return this->answer(new NETDATA(NGWINAMP_REQ_PLADDFILES, paths.size(), 0, 0, 0.0, writer));
}
bool NGWINAMPCLIENT::request_pl_delfiles(const vector<dword> &indexes) {
	NGWRITER writer;

	for (dword i = 0; i < indexes.size() ; i++) {
		writer.writeDword(indexes[i]);
	}
	return this->answer(new NETDATA(NGWINAMP_REQ_PLDELFILES, indexes.size(), 0, 0, 0.0, writer));
}
bool NGWINAMPCLIENT::request_pl_getnames(const long index) {
	if (index >= 0) {
		return this->answer(new NETDATA(NGWINAMP_REQ_PLGETNAMES, index, 0, 0, 0.0));
	}
	return this->answer(new NETDATA(NGWINAMP_REQ_PLGETNAMES, NGWINAMP_ALL, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_pl_getfiles(const long index) {
	if (index >= 0) {
		return this->answer(new NETDATA(NGWINAMP_REQ_PLGETFILES, index, 0, 0, 0.0));
	}
	return this->answer(new NETDATA(NGWINAMP_REQ_PLGETFILES, NGWINAMP_ALL, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_pl_getpos(void) {
	return this->answer(new NETDATA(NGWINAMP_REQ_PLGETPOS, 0, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_pl_setpos(const dword index) {
	return this->answer(new NETDATA(NGWINAMP_REQ_PLSETPOS, index, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_pl_setrepeat(const bool repeat) {
	if (repeat) {
		return this->answer(new NETDATA(NGWINAMP_REQ_PLSETREPEAT, NGWINAMP_ALL, 0, 0, 0.0));
	}
	return this->answer(new NETDATA(NGWINAMP_REQ_PLSETREPEAT, NGWINAMP_NONE, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_pl_setshuffle(const bool shuffle) {
	if (shuffle) {
		return this->answer(new NETDATA(NGWINAMP_REQ_PLSETSHUFFLE, NGWINAMP_ALL, 0, 0, 0.0));
	}
	return this->answer(new NETDATA(NGWINAMP_REQ_PLSETSHUFFLE, NGWINAMP_NONE, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_pl_clear(void) {
	return this->answer(new NETDATA(NGWINAMP_REQ_PLCLEAR, 0, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_pl_removedead(void) {
	return this->answer(new NETDATA(NGWINAMP_REQ_PLDELDEADFILES, 0, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_pl_sortbyname(void) {
	return this->answer(new NETDATA(NGWINAMP_REQ_PLSORTBYNAME, 0, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_pl_sortbypath(void) {
	return this->answer(new NETDATA(NGWINAMP_REQ_PLSORTBYPATH, 0, 0, 0, 0.0));
}
bool NGWINAMPCLIENT::request_pl_randomize(void) {
	return this->answer(new NETDATA(NGWINAMP_REQ_PLRANDOMIZE, 0, 0, 0, 0.0));
}

bool NGWINAMPCLIENT::request_bw_getdirectories(const string &path) {
	NGWRITER writer;

	writer.writeWordString(path);
	return this->answer(new NETDATA(NGWINAMP_REQ_BWGETDIRECTORIES, 0, 0, 0, 0.0, writer));
}
bool NGWINAMPCLIENT::request_bw_getfiles(const string &path) {
	NGWRITER writer;

	writer.writeWordString(path);
	return this->answer(new NETDATA(NGWINAMP_REQ_BWGETFILES, 0, 0, 0, 0.0, writer));
}



bool NGWINAMPCLIENT::init(void) {
	NGLOCKER locker(this);

	// init data
	ResetEvent(this->hquit);
	SetEvent(this->hrunning);
	this->timer.start();
	this->defaultflags = NGWINAMP_FILTER_ALLOWZZIP;
	this->access = NGWINAMPUSER_ACCESS_NONE;
	this->timeout = 0;
	this->eof = false;

	// resolve server address
	SOCKADDR_IN	addr;
	PHOSTENT	phost = gethostbyname(this->cfg_address.c_str());

	if (phost == NULL) {
		DEBUGWRITE("NGWINAMPCLIENT::init() cannot resolve server address !");
		locker.release();
		this->clientwnd->onnet_connected(false, "Networking error", "Cannot resolve server address !");
		return false;
	}
	if (phost->h_length != 4) {
		DEBUGWRITE("NGWINAMPCLIENT::init() invalid server address (not IPv4) !");
		locker.release();
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
		locker.release();
		this->clientwnd->onnet_connected(false, "Networking error", "Socket failure !");
		return false;
	}
	if (connect(this->sclient, (const struct sockaddr*)&addr, sizeof(SOCKADDR_IN)) != 0) {
		DEBUGWRITE("NGWINAMPCLIENT::init() cannot connect to server !");
		locker.release();
		this->clientwnd->onnet_connected(false, "Networking error", "Cannot connect to server,\r\nconnection refused !");
		return false;
	}
	if (ioctlsocket(this->sclient, FIONBIO, &enabled) != 0) {
		DEBUGWRITE("NGWINAMPCLIENT::init() cannot change ioctl !");
		locker.release();
		this->clientwnd->onnet_connected(false, "Networking error", "Socket failure !");
		return false;
	}
	DEBUGWRITE("NGWINAMPCLIENT::init()");

	locker.release();
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
	this->access = NGWINAMPUSER_ACCESS_NONE;

	// free data
	ResetEvent(this->hquit);
	ResetEvent(this->hrunning);
	this->hthread = NULL;

	DEBUGWRITE("NGWINAMPCLIENT::free()");

	locker.release();
	this->clientwnd->onnet_disconnect();
}


bool NGWINAMPCLIENT::main(void) {
	// check connection
	if (this->recvmsg()) {
		return this->sendmsg();
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
			int error = WSAGetLastError();

			switch (error) {
			case WSAEWOULDBLOCK:
				break;

			default:
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
		} else {
			Sleep(5);
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
