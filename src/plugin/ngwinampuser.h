// ngwinampuser.h
#ifndef _NGWINAMPUSER_H_INCLUDE_
#define _NGWINAMPUSER_H_INCLUDE_


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
