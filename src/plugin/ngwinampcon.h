// ngwinampcon.h
#ifndef _NGWINAMPCON_H_INCLUDE_
#define _NGWINAMPCON_H_INCLUDE_


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
	NETSERVERSNAPSHOT	snapshot;

	// requests
	vector<NETDATA*>	requests;
	NGBUFFER			recvbuffer;

	// answers
	vector<NETDATA*>	answers;
	NGBUFFER			sendbuffer;


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

	bool	 isclosed(void);
	void	 close(void);
	void	 shutdown(void);
};


#endif //_NGWINAMPCON_H_INCLUDE_
