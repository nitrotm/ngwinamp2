// netauth.h
#ifndef _NETAUTH_H_INCLUDE_
#define _NETAUTH_H_INCLUDE_


/**
  * Authentication codes
  *
  */
#define NGWINAMP_AUTH_SUCCESS		0x0
#define NGWINAMP_AUTH_FAILURE		0x1
#define NGWINAMP_AUTH_NOTDONE		0x2
#define NGWINAMP_AUTH_TOOMANYCON	0x3


/**
  * Authentication access rights
  *
  */
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


/**
  * Authentication answer (NGWINAMP_ANS_AUTH)
  *
  * note: obsolete
  *
  */
typedef struct {
	dword			periodecount;	// note: unused
	double			periodetime;	// note: unused
	double			timeout;
	dword			reserved;
} NETAUTH;


/**
  * Authentication answer (NGWINAMP_ANS_AUTH_EX)
  *
  */
typedef struct {
	// server part
	byte			vmajor;
	byte			vminor;
	// client part
	dword			access;
	dword			maxconnection;
	double			timeout;
} NETAUTHEX;


#endif //_NETAUTH_H_INCLUDE_
