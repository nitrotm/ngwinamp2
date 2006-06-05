// ngwinampclient.h
#ifndef _NGWINAMPCLIENT_H_INCLUDE_
#define _NGWINAMPCLIENT_H_INCLUDE_


class NGMainWnd;
class NGWINAMPCLIENT;


int WINAPI NGWINAMPCLIENT_thread(NGWINAMPCLIENT *pclient);


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

	bool process(void);
	bool answer(NETDATA *panswer);


public:
	NGWINAMPCLIENT(NGMainWnd *clientwnd);
	virtual ~NGWINAMPCLIENT();

	bool isrunning(void);
	bool isquit(void);

	bool start(const string &address, const word port);
	bool stop(void);
	void shutdown(void);

	bool authenticate(const string &username, const string &password);
};


#endif //_NGWINAMPCLIENT_H_INCLUDE_
