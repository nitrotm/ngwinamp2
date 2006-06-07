// ngwinampserver.h
#ifndef _NGWINAMPSERVER_H_INCLUDE_
#define _NGWINAMPSERVER_H_INCLUDE_


class NGWINAMPSERVER;
class NGWINAMPUSER;
class NGWINAMPCON;


/**
  * Server manager
  *
  */
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
	bool					cfg_allowzzip;
	double					cfg_sharerefresh;

	// server states
	SOCKET					swait;
	vector<NGWINAMPCON*>	connections;

	// client states
	vector<NGWINAMPUSER*>	users;

	// sharing
	NGTIMER					sharetimer;
	FSRoot					shares;

	// global states
	HANDLE					hthread;
	HANDLE					hrunning;
	HANDLE					hquit;


	void readcfg(const string &filename);
	void appendshare(FSRoot *parent, const CFGNode &share);
	void savecfg(const string &filename);

	bool init(void);
	void free(void);

	void main(void);
	void accept(void);
	void gc(void);

	NGWINAMPUSER* finduser(const string &username);

	bool authenticate(NGWINAMPCON *pconnection, NETDATA *prequest);


public:
	NGWINAMPSERVER(HWND hwndplugin);
	virtual ~NGWINAMPSERVER();

	bool isrunning(void);
	bool isquit(void);

	bool start(void);
	bool stop(void);

	const FSNode* findshare(const string &path);
	vector<string> getfilepaths(const string &username, const string &path);
	vector<string> getfilepaths(const string &username, const FSNode *pnode, const vector<string> childs);
};


#endif //_NGWINAMPSERVER_H_INCLUDE_
