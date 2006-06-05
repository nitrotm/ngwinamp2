// ngwinampuser.h
#ifndef _NGWINAMPUSER_H_INCLUDE_
#define _NGWINAMPUSER_H_INCLUDE_


#define NGWINAMPUSER_ACCESS_NONE		0x00000000
#define NGWINAMPUSER_ACCESS_READ		0x00000001
#define NGWINAMPUSER_ACCESS_WRITE		0x00000002
#define NGWINAMPUSER_ACCESS_ADMIN		0x00000004
#define NGWINAMPUSER_ACCESS_PL_ADD		0x00000008
#define NGWINAMPUSER_ACCESS_PL_SET		0x00000010
#define NGWINAMPUSER_ACCESS_PL_DEL		0x00000020
#define NGWINAMPUSER_ACCESS_PL_CTRL		0x00000040
#define NGWINAMPUSER_ACCESS_SN_BACK		0x00000080
#define NGWINAMPUSER_ACCESS_SN_PLAY		0x00000100
#define NGWINAMPUSER_ACCESS_SN_PAUSE	0x00000200
#define NGWINAMPUSER_ACCESS_SN_STOP		0x00000400
#define NGWINAMPUSER_ACCESS_SN_NEXT		0x00000800
#define NGWINAMPUSER_ACCESS_SN_VOLUME	0x00001000
#define NGWINAMPUSER_ACCESS_SN_PAN		0x00002000
#define NGWINAMPUSER_ACCESS_SN_POS		0x00004000


class NGWINAMPUSER : public NGLOCK {
protected:
	friend class NGWINAMPSERVER;

	// server & connections
	NGWINAMPSERVER			*pserver;
	vector<NGWINAMPCON*>	connections;

	// credentials
	vector<NETADDR>			denied;
	vector<NETADDR>			allowed;
	string					username;
	string					password;
	dword					access;
	dword					maxcon;
	double					timeout;

	// statistics
	dword					byte_in;
	dword					byte_out;


	bool   exists(NGWINAMPCON *pconnection);
	void   add(NGWINAMPCON *pconnection);

	void   main(void);
	void   gc(void);
	bool   process(NGWINAMPCON *pconnection, NETDATA *prequest);

	dword  authenticate(const string &password, const SOCKADDR_IN &address);


public:
	NGWINAMPUSER(NGWINAMPSERVER *pserver, const string &username, const string &password);
	virtual ~NGWINAMPUSER();

	void   close(void);

	dword  parsepolicies(CFGNode &access);
	void   setpolicies(dword access, dword maxcon, double timeout);
	void   deny(const NETADDR &addr);
	void   allow(const NETADDR &addr);

	string getusername(void);
	string getpassword(void);
	dword  getaccess(void);
	dword  getmaxcon(void);
	double gettimeout(void);
	bool   hasaccess(dword what);
	bool   canread(void);
	bool   canwrite(void);
	bool   canadmin(void);
};


#endif //_NGWINAMPUSER_H_INCLUDE_
