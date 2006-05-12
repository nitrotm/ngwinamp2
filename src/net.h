// net.h
#ifndef _NET_H_INCLUDE_
#define _NET_H_INCLUDE_


// socket structures
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct hostent     SOCKHOSTENT;


// authentification code
#define NGWINAMP_AUTH_SUCCESS		0x0
#define NGWINAMP_AUTH_FAILURE		0x1
#define NGWINAMP_AUTH_NOTDONE		0x2
#define NGWINAMP_AUTH_TOOMANYCON	0x3


// commandes (requests)
#define NGWINAMP_REQ_NONE			0
 // authentification
#define NGWINAMP_REQ_AUTH			1
#define NGWINAMP_REQ_AUTH_EX		2
 // son
#define NGWINAMP_REQ_PREV			11
#define NGWINAMP_REQ_PLAY			12
#define NGWINAMP_REQ_PAUSE			13
#define NGWINAMP_REQ_STOP			14
#define NGWINAMP_REQ_NEXT			15
#define NGWINAMP_REQ_GETVOLUME		21
#define NGWINAMP_REQ_SETVOLUME		22
#define NGWINAMP_REQ_GETPAN			23
#define NGWINAMP_REQ_SETPAN			24
#define NGWINAMP_REQ_GETPOS			25
#define NGWINAMP_REQ_SETPOS			26
 // playlist
#define NGWINAMP_REQ_PLCLEAR		100
#define NGWINAMP_REQ_PLGETNAMES		101
#define NGWINAMP_REQ_PLGETFILES		103
#define NGWINAMP_REQ_PLSETFILES		104
#define NGWINAMP_REQ_PLDELFILES		105
#define NGWINAMP_REQ_PLADDFILES		106
#define NGWINAMP_REQ_PLMOVEFILES	107
#define NGWINAMP_REQ_PLGETPOS		111
#define NGWINAMP_REQ_PLSETPOS		112
#define NGWINAMP_REQ_PLGETSHUFFLE	121
#define NGWINAMP_REQ_PLSETSHUFFLE	122
#define NGWINAMP_REQ_PLGETREPEAT	123
#define NGWINAMP_REQ_PLSETREPEAT	124
#define NGWINAMP_REQ_PLRANDOMIZE	130
#define NGWINAMP_REQ_PLSORTBYNAME	131
#define NGWINAMP_REQ_PLSORTBYPATH	132
#define NGWINAMP_REQ_PLDELDEADFILES	133
 // browsing
#define NGWINAMP_REQ_BWGETROOTS		200
#define NGWINAMP_REQ_BWGETLIST		201
 // snapshots
#define NGWINAMP_REQ_SNAPSHOT		300



// réponses (answers)
#define NGWINAMP_ANS_NONE			0
 // authentification
#define NGWINAMP_ANS_AUTH			1
#define NGWINAMP_ANS_AUTH_EX		2
 // son
#define NGWINAMP_ANS_VOLUME			21
#define NGWINAMP_ANS_PAN			23
#define NGWINAMP_ANS_POS			25
 // playlist
#define NGWINAMP_ANS_PLNAMES		101
#define NGWINAMP_ANS_PLFILES		103
#define NGWINAMP_ANS_PLBROWSE		107
#define NGWINAMP_ANS_PLPOS			111
#define NGWINAMP_ANS_PLSHUFFLE		121
#define NGWINAMP_ANS_PLREPEAT		123
 // browsing
#define NGWINAMP_ANS_BWROOTS		200
#define NGWINAMP_ANS_BWLIST			201
 // snapshots
#define NGWINAMP_ANS_SNAPSHOT		300 // TODO


// authentication answer (NGWINAMP_ANS_AUTH)
typedef struct {
	dword			periodecount;
	double			periodetime;
	double			timeout;
	dword			reserved;
} NETAUTH;

// authentication answer (NGWINAMP_ANS_AUTH_EX)
typedef struct {
	// server part
	byte			vmajor;
	byte			vminor;
	bool			allowzzip;
	// client part
	dword			access;
	dword			maxconnection;
	double			timeout;
} NETAUTHEX;


// snapshot answer (NGWINAMP_ANS_SNAPSHOT)
#define NGWINAMP_SNAPSHOT_NONE					0x00000000
#define NGWINAMP_SNAPSHOT_SN_VOLUME				0x00000001
#define NGWINAMP_SNAPSHOT_SN_PAN				0x00000002
#define NGWINAMP_SNAPSHOT_SN_POSMS				0x00000004
#define NGWINAMP_SNAPSHOT_SN_LENGTH				0x00000008
#define NGWINAMP_SNAPSHOT_PL_POS				0x00000010
#define NGWINAMP_SNAPSHOT_PL_LENGTH				0x00000020
#define NGWINAMP_SNAPSHOT_PL_SHUFFLE			0x00000040
#define NGWINAMP_SNAPSHOT_PL_REPEAT				0x00000080
#define NGWINAMP_SNAPSHOT_ALL					0xFFFFFFFF

#define NGWINAMP_SNAPSHOT_HASFLAG(pcon, flag)	(((pcon)->snapshot.mflags & (flag)) != 0)

typedef struct {
	dword  mflags;

	double sn_volume;
	double sn_pan;
	dword  sn_posms;
	dword  sn_length;
	dword  pl_pos;
	dword  pl_length;
	bool   pl_shuffle;
	bool   pl_repeat;
} NETSNAPSHOT;


#endif //_NET_H_INCLUDE_
