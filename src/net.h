// net.h
#ifndef _NET_H_INCLUDE_
#define _NET_H_INCLUDE_


/**
  * Commands (requests)
  *
  */
#define NGWINAMP_REQ_NONE				0
 // authentication
#define NGWINAMP_REQ_AUTH				1 // note: obsolete
#define NGWINAMP_REQ_AUTH_EX			2
 // sound
#define NGWINAMP_REQ_PREV				11
#define NGWINAMP_REQ_PLAY				12
#define NGWINAMP_REQ_PAUSE				13
#define NGWINAMP_REQ_STOP				14
#define NGWINAMP_REQ_NEXT				15
#define NGWINAMP_REQ_GETVOLUME			21
#define NGWINAMP_REQ_SETVOLUME			22
#define NGWINAMP_REQ_GETPAN				23
#define NGWINAMP_REQ_SETPAN				24
#define NGWINAMP_REQ_GETPOS				25
#define NGWINAMP_REQ_SETPOS				26
 // playlist
#define NGWINAMP_REQ_PLCLEAR			100
#define NGWINAMP_REQ_PLGETNAMES			101
#define NGWINAMP_REQ_PLGETFILES			103
#define NGWINAMP_REQ_PLSETFILES			104
#define NGWINAMP_REQ_PLDELFILES			105
#define NGWINAMP_REQ_PLADDFILES			106
#define NGWINAMP_REQ_PLMOVEFILES		107
#define NGWINAMP_REQ_PLGETPOS			111
#define NGWINAMP_REQ_PLSETPOS			112
#define NGWINAMP_REQ_PLGETSHUFFLE		121
#define NGWINAMP_REQ_PLSETSHUFFLE		122
#define NGWINAMP_REQ_PLGETREPEAT		123
#define NGWINAMP_REQ_PLSETREPEAT		124
#define NGWINAMP_REQ_PLRANDOMIZE		130
#define NGWINAMP_REQ_PLSORTBYNAME		131
#define NGWINAMP_REQ_PLSORTBYPATH		132
#define NGWINAMP_REQ_PLDELDEADFILES		133
#define NGWINAMP_REQ_PLADDVOTE			150 // note: TODO
 // browsing
#define NGWINAMP_REQ_BWGETROOTS			200 // note: obsolete
#define NGWINAMP_REQ_BWGETLIST			201 // note: obsolete
#define NGWINAMP_REQ_BWGETDIRECTORIES	202 // note: TODO
#define NGWINAMP_REQ_BWGETFILES			203 // note: TODO
 // snapshots
#define NGWINAMP_REQ_GETSNAPSHOT		300 // note: obsolete
#define NGWINAMP_REQ_GETSNAPSHOT_EX		301 // note: TODO
 // administration
#define NGWINAMP_REQA_GETSHARES			500 // note: TODO
#define NGWINAMP_REQA_SETSHARE			501 // note: TODO
#define NGWINAMP_REQA_ADDSHARE			502 // note: TODO
#define NGWINAMP_REQA_DELSHARE			503 // note: TODO
#define NGWINAMP_REQA_GETUSERS			510 // note: TODO
#define NGWINAMP_REQA_SETUSER			511 // note: TODO
#define NGWINAMP_REQA_ADDUSER			512 // note: TODO
#define NGWINAMP_REQA_DELUSER			513 // note: TODO
#define NGWINAMP_REQA_GETCLIENTS		520 // note: TODO
#define NGWINAMP_REQA_KILLCLIENT		521 // note: TODO
#define NGWINAMP_REQA_RESTART			550 // note: TODO


/**
  * Commands (answers)
  *
  */
#define NGWINAMP_ANS_NONE				0
 // authentication
#define NGWINAMP_ANS_AUTH				1 // note: obsolete
#define NGWINAMP_ANS_AUTH_EX			2
 // sound
#define NGWINAMP_ANS_VOLUME				21
#define NGWINAMP_ANS_PAN				23
#define NGWINAMP_ANS_POS				25
 // playlist
#define NGWINAMP_ANS_PLNAMES			101
#define NGWINAMP_ANS_PLFILES			103
#define NGWINAMP_ANS_PLBROWSE			107 // note: invalid
#define NGWINAMP_ANS_PLPOS				111
#define NGWINAMP_ANS_PLSHUFFLE			121
#define NGWINAMP_ANS_PLREPEAT			123
 // browsing
#define NGWINAMP_ANS_BWROOTS			200 // note: obsolete
#define NGWINAMP_ANS_BWLIST				201 // note: obsolete
#define NGWINAMP_ANS_BWDIRECTORIES		202 // note: TODO
#define NGWINAMP_ANS_BWFILES			203 // note: TODO
 // snapshots
#define NGWINAMP_ANS_SNAPSHOT_EX		301 // note: TODO
 // administration
#define NGWINAMP_ANSA_SHARES			500 // note: TODO
#define NGWINAMP_ANSA_USERS				510 // note: TODO
#define NGWINAMP_ANSA_CLIENTS			520 // note: TODO


/**
  * Compression & filters
  *
  */
#define NGWINAMP_FILTER_NONE			0x00000000
#define NGWINAMP_FILTER_ZZIP			0x00000001
#define NGWINAMP_FILTER_ALLOWZZIP		0x00000002


/**
  * Network header
  *
  */
typedef struct {
	dword			code;
	dword			param1;
	dword			param2;
	dword			flags;
	double			param3;
	dword			size;
	dword			size2;
} NETHDR;


#endif //_NET_H_INCLUDE_
