// ngwinampserver.h
#ifndef _NGWINAMPSERVER_H_INCLUDE_
#define _NGWINAMPSERVER_H_INCLUDE_


class NGWINAMPSERVER;
class NGWINAMPUSER;
class NGWINAMPCON;

int WINAPI NGWINAMPSERVER_thread(NGWINAMPSERVER *pserver);


class NGWINAMPSERVER : public NGWINAMP {
protected:
	friend int WINAPI NGWINAMPSERVER_thread(NGWINAMPSERVER *pserver);


	// configuration
	CFG						cfg;
	bool					cfg_enabled;
	string					cfg_address;
	word					cfg_port;
	dword					cfg_connection;
	dword					cfg_timeout;
	dword					cfg_buffersize;

	// server states
	SOCKET					swait;
	vector<NGWINAMPCON*>	connections;

	// client states
	vector<NGWINAMPUSER*>	users;

	// sharing
	FSNode					shares;

	// global states
	HANDLE					hthread;
	HANDLE					hrunning;
	HANDLE					hquit;


	void readcfg(const string &filename);
	void savecfg(const string &filename);

	bool init(void);
	void free(void);

	void main(void);
	void accept(void);
	void gc(void);

	NGWINAMPUSER* find(const string &username);

	bool authenticate(NGWINAMPCON *pconnection, NETDATA *prequest);


public:
	NGWINAMPSERVER(HWND hwndplugin);
	virtual ~NGWINAMPSERVER();

	bool isrunning();
	bool isquit();

	bool start();
	bool stop();
};


#endif //_NGWINAMPSERVER_H_INCLUDE_
