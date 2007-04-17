// ngwinampclient.h
#ifndef _NGWINAMPCLIENT_H_INCLUDE_
#define _NGWINAMPCLIENT_H_INCLUDE_


class NGMainWnd;
class NGWINAMPCLIENT;


/**
  * Connection manager
  *
  */
class NGWINAMPCLIENT : public NGLOCK {
protected:
	friend int WINAPI NGWINAMPCLIENT_thread(NGWINAMPCLIENT *pclient);


	// window
	NGMainWnd			*clientwnd;

	// configuration
	string				cfg_address;
	word				cfg_port;
	dword				cfg_timeout;
	dword				cfg_buffersize;
	bool				cfg_allowzzip;

	// client states
	SOCKET				sclient;
	NGTIMER				timer;
	dword				defaultflags;
	dword				access;
	double				timeout;
	bool				eof;

	// global states
	HANDLE				hthread;
	HANDLE				hrunning;
	HANDLE				hquit;

	// requests
	vector<NETDATA*>	requests;
	NGBUFFER			recvbuffer;

	// answers
	vector<NETDATA*>	answers;
	NGBUFFER			sendbuffer;


	bool init(void);
	void free(void);

	bool main(void);

	bool recvmsg(void);
	bool sendmsg(void);

	bool answer(NETDATA *panswer);


public:
	NGWINAMPCLIENT(NGMainWnd *clientwnd);
	virtual ~NGWINAMPCLIENT();

	bool isrunning(void);
	bool isquit(void);

	bool start(const string &address, const word port);
	bool stop(void);
	void shutdown(void);

	bool hasaccess(const dword access);
	bool process(void);


	bool authenticate(const string &username, const string &password);

	bool request_snapshot(const double timeout);

	bool request_sn_prev(void);
	bool request_sn_play(void);
	bool request_sn_pause(void);
	bool request_sn_stop(void);
	bool request_sn_next(void);
	bool request_sn_setvolume(const double volume);
	bool request_sn_setpan(const double pan);
	bool request_sn_setpos(const double pos);

	bool request_pl_addfiles(const dword index, const vector<string> &paths);
	bool request_pl_delfiles(const vector<dword> &indexes);
	bool request_pl_getnames(const long index = -1);
	bool request_pl_getfiles(const long index = -1);
	bool request_pl_getpos(void);
	bool request_pl_setpos(const dword index);
	bool request_pl_setrepeat(const bool repeat);
	bool request_pl_setshuffle(const bool shuffle);
	bool request_pl_clear(void);
	bool request_pl_removedead(void);
	bool request_pl_sortbyname(void);
	bool request_pl_sortbypath(void);
	bool request_pl_randomize(void);

	bool request_bw_getdirectories(const string &path);
	bool request_bw_getfiles(const string &path);
};


#endif //_NGWINAMPCLIENT_H_INCLUDE_
