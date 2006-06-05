// ngwinampserver.h
#ifndef _NGWINAMPSERVER_H_INCLUDE_
#define _NGWINAMPSERVER_H_INCLUDE_


class NGWINAMPSERVER;
class NGWINAMPUSER;
class NGWINAMPCON;

int WINAPI NGWINAMPSERVER_thread(NGWINAMPSERVER *pserver);


#define NGWINAMP_SNAPSHOT_SETFLAG(pcon, flag)	(pcon)->snapshot.mflags |= (flag)
#define NGWINAMP_SNAPSHOT_RESETFLAG(pcon, flag)	(pcon)->snapshot.mflags &= ~(flag)

#define NGWINAMP_SNAPSHOT_SN_SETVOLUME(pcon, value)						\
	if ((pcon)->snapshot.sn_volume != value) {							\
		(pcon)->snapshot.sn_volume = value;								\
		NGWINAMP_SNAPSHOT_SETFLAG(pcon, NGWINAMP_SNAPSHOT_SN_VOLUME);	\
	}
#define NGWINAMP_SNAPSHOT_SN_SETPAN(pcon, value)						\
	if ((pcon)->snapshot.sn_pan != value) {								\
		(pcon)->snapshot.sn_pan = value;								\
		NGWINAMP_SNAPSHOT_SETFLAG(pcon, NGWINAMP_SNAPSHOT_SN_PAN);		\
	}
#define NGWINAMP_SNAPSHOT_SN_SETPOSMS(pcon, value)						\
	if ((pcon)->snapshot.sn_posms != value) {							\
		(pcon)->snapshot.sn_posms = value;								\
		NGWINAMP_SNAPSHOT_SETFLAG(pcon, NGWINAMP_SNAPSHOT_SN_POSMS);	\
	}
#define NGWINAMP_SNAPSHOT_SN_SETLENGTH(pcon, value)						\
	if ((pcon)->snapshot.sn_length != value) {							\
		(pcon)->snapshot.sn_length = value;								\
		NGWINAMP_SNAPSHOT_SETFLAG(pcon, NGWINAMP_SNAPSHOT_SN_LENGTH);	\
	}

#define NGWINAMP_SNAPSHOT_PL_SETPOS(pcon, value)						\
	if ((pcon)->snapshot.pl_pos != value) {								\
		(pcon)->snapshot.pl_pos = value;								\
		NGWINAMP_SNAPSHOT_SETFLAG(pcon, NGWINAMP_SNAPSHOT_PL_POS);		\
	}
#define NGWINAMP_SNAPSHOT_PL_SETLENGTH(pcon, value)						\
	if ((pcon)->snapshot.pl_length != value) {							\
		(pcon)->snapshot.pl_length = value;								\
		NGWINAMP_SNAPSHOT_SETFLAG(pcon, NGWINAMP_SNAPSHOT_PL_LENGTH);	\
	}
#define NGWINAMP_SNAPSHOT_PL_SETSHUFFLE(pcon, value)					\
	if ((pcon)->snapshot.pl_shuffle != value) {							\
		(pcon)->snapshot.pl_shuffle = value;							\
		NGWINAMP_SNAPSHOT_SETFLAG(pcon, NGWINAMP_SNAPSHOT_PL_SHUFFLE);	\
	}
#define NGWINAMP_SNAPSHOT_PL_SETREPEAT(pcon, value)						\
	if ((pcon)->snapshot.pl_repeat != value) {							\
		(pcon)->snapshot.pl_repeat = value;								\
		NGWINAMP_SNAPSHOT_SETFLAG(pcon, NGWINAMP_SNAPSHOT_PL_REPEAT);	\
	}


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
