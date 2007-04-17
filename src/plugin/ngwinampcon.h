// ngwinampcon.h
#ifndef _NGWINAMPCON_H_INCLUDE_
#define _NGWINAMPCON_H_INCLUDE_


/**
  * Client connection manager
  *
  */
class NGWINAMPCON : public NGLOCK {
protected:
	friend class NGWINAMPSERVER;
	friend class NGWINAMPUSER;

	// connection
	SOCKADDR_IN			address;
	SOCKET				s;
	NGTIMER				timer;
	double				timeout;
	dword				defaultflags;
	bool				eof;

	// winamp state
	NETSNAPSHOT			snapshot;
	NGTIMER				lastsnapshot;
	double				snapshotinverval;

	// requests
	vector<NETDATA*>	requests;
	NGBUFFER			recvbuffer;

	// answers
	vector<NETDATA*>	answers;
	NGBUFFER			sendbuffer;

	// statistics
	dword				byte_in;
	dword				byte_out;


	bool	 main(void);
	bool	 recvmsg(void);
	bool	 sendmsg(void);

	NETDATA* request(void);
	bool	 answer(NETDATA *panswer);


public:
	NGWINAMPCON(SOCKET s, const SOCKADDR_IN &address, double timeout);
	virtual ~NGWINAMPCON();

	void	 setflags(dword flags);
	void	 settimeout(double timeout);
	void	 setsnapshot(double inverval);
	bool	 checksnapshot(void);

	dword	 getbytein(void);
	dword	 getbyteout(void);

	bool	 isclosed(void);
	void	 close(void);
	void	 shutdown(void);
};


#endif //_NGWINAMPCON_H_INCLUDE_
