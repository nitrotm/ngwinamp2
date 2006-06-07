// netsnapsot.h
#ifndef _NETSNAPSHOT_H_INCLUDE_
#define _NETSNAPSHOT_H_INCLUDE_


/**
  * Snapshot flags (NGWINAMP_ANS_SNAPSHOT)
  *
  */
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


/**
  * Winamp snapshot values
  *
  */
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


/**
  * Snapshot helpers
  *
  */
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


#endif //_NETSNAPSHOT_H_INCLUDE_
