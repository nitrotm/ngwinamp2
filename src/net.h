// net.h
#ifndef _NET_H_INCLUDE_
#define _NET_H_INCLUDE_



// constantes génériques
#define NGWINAMP_NONE				0x0
#define NGWINAMP_ALL				0xFFFFFFFF


// authentification
#define NGWINAMP_AUTH_SUCCESS		0x0
#define NGWINAMP_AUTH_FAILURE		0x1
#define NGWINAMP_AUTH_NOTDONE		0x2
#define NGWINAMP_AUTH_TOOMANYCON	0x3


// compression & filters
#define NGWINAMP_FILTER_NONE		0x00000000
#define NGWINAMP_FILTER_ZZIP		0x00000001
#define NGWINAMP_FILTER_ALLOWZZIP	0x00000002


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
#define NGWINAMP_REQ_GETSNAPSHOT	300



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


// snapshot de l'état de winamp
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
} NETSERVERSNAPSHOT;

// informations sur le serveur
typedef struct {
	dword			periodecount;
	double			periodetime;
	double			timeout;
	dword			reserved;
} NETSERVERINFO;

// informations sur le serveur
typedef struct {
	// server part
	dword			periodecount;
	double			periodetime;
	double			timeout;
	// client part
	dword			access;
} NETSERVERINFOEX;


// adresse réseau
class NETADDR {
public:
	byte ip[4];
	byte mask[4];

	NETADDR() {
		this->ip[0] = this->ip[1] = this->ip[2] = this->ip[3] = 0;
		this->mask[0] = this->mask[1] = this->mask[2] = this->mask[3] = 0x0;
	}
	NETADDR(byte ip1, byte ip2, byte ip3, byte ip4) {
		this->ip[0] = ip1;
		this->ip[1] = ip2;
		this->ip[2] = ip3;
		this->ip[3] = ip4;
		this->mask[0] = this->mask[1] = this->mask[2] = this->mask[3] = 0xff;
	}
	NETADDR(byte ip1, byte ip2, byte ip3, byte ip4, byte mask1, byte mask2, byte mask3, byte mask4) {
		this->ip[0] = ip1;
		this->ip[1] = ip2;
		this->ip[2] = ip3;
		this->ip[3] = ip4;
		this->mask[0] = mask1;
		this->mask[1] = mask2;
		this->mask[2] = mask3;
		this->mask[3] = mask4;
	}
	NETADDR(byte ip[4]) {
		this->ip[0] = ip[0];
		this->ip[1] = ip[1];
		this->ip[2] = ip[2];
		this->ip[3] = ip[3];
		this->mask[0] = this->mask[1] = this->mask[2] = this->mask[3] = 0xff;
	}
	NETADDR(byte ip[4], byte mask[4]) {
		this->ip[0] = ip[0];
		this->ip[1] = ip[1];
		this->ip[2] = ip[2];
		this->ip[3] = ip[3];
		this->mask[0] = mask[0];
		this->mask[1] = mask[1];
		this->mask[2] = mask[2];
		this->mask[3] = mask[3];
	}
	NETADDR(const NETADDR &src) {
		this->ip[0] = src.ip[0];
		this->ip[1] = src.ip[1];
		this->ip[2] = src.ip[2];
		this->ip[3] = src.ip[3];
		this->mask[0] = src.mask[0];
		this->mask[1] = src.mask[1];
		this->mask[2] = src.mask[2];
		this->mask[3] = src.mask[3];
	}

	static vector<NETADDR> parse(const string &value) {
		vector<NETADDR> items;
		vector<string>	values = strsplit(value, " ", 0);

		for (dword i = 0; i < values.size(); i++) {
			string value = strtrim(values[i]);

			if (value.length() > 0) {
				vector<string> item = strsplit(value, "/", 0);

				if (item.size() == 1) {
					vector<string> ip = strsplit(item[0], ".", 7);

					if (ip.size() == 7) {
						items.push_back(NETADDR(atoi(ip[0].c_str()), atoi(ip[2].c_str()), atoi(ip[4].c_str()), atoi(ip[6].c_str())));
					}
				} else if (item.size() == 3) {
					vector<string> ip = strsplit(item[0], ".", 7);

					if (ip.size() == 7) {
						vector<string> mask = strsplit(item[2], ".", 7);

						if (mask.size() == 1) {
							dword masku = atoi(mask[0].c_str());
							dword maskt = 0;

							for (dword i = 0; i < masku; i++) {
								maskt |= 1 << i;
							}
							items.push_back(NETADDR(atoi(ip[0].c_str()), atoi(ip[2].c_str()), atoi(ip[4].c_str()), atoi(ip[6].c_str()), 
								(byte)(maskt >> 0) & 0xFF, (byte)(maskt >> 8) & 0xFF, (byte)(maskt >> 16) & 0xFF, (byte)(maskt >> 24) & 0xFF));
						} else if (mask.size() == 7) {
							items.push_back(NETADDR(atoi(ip[0].c_str()), atoi(ip[2].c_str()), atoi(ip[4].c_str()), atoi(ip[6].c_str()),
								atoi(mask[0].c_str()), atoi(mask[2].c_str()), atoi(mask[4].c_str()), atoi(mask[6].c_str())));
						}
					}
				}
			}
		}
		return items;
	}
};


// header réseau
typedef struct {
	dword			code;
	dword			param1;
	dword			param2;
	dword			flags;
	double			param3;
	dword			size;
	dword			size2;
} NETHDR;


// transferts de données
class NETDATA {
public:
	NETHDR		hdr;
	NGBUFFER	buffer;


	NETDATA(const NETHDR &hdr, const NGBUFFER &data) : buffer(data) {
		memcpy(&this->hdr, &hdr, sizeof(NETHDR));
	}
	NETDATA(dword code, dword param1, dword param2, dword flags, double param3) {
		memset(&this->hdr, 0, sizeof(NETHDR));
		this->hdr.code = code;
		this->hdr.param1 = param1;
		this->hdr.param2 = param2;
		this->hdr.flags = flags;
		this->hdr.param3 = param3;
	}
	NETDATA(dword code, dword param1, dword param2, dword flags, double param3, void *ptr, dword size) {
		memset(&this->hdr, 0, sizeof(NETHDR));
		this->hdr.code = code;
		this->hdr.param1 = param1;
		this->hdr.param2 = param2;
		this->hdr.flags = flags;
		this->hdr.param3 = param3;
		this->hdr.size = size;
		this->buffer.alloc(size);
		this->buffer.write(ptr, 0, size);
	}
	NETDATA(dword code, dword param1, dword param2, dword flags, double param3, const NGBUFFER &data) : buffer(data) {
		memset(&this->hdr, 0, sizeof(NETHDR));
		this->hdr.code = code;
		this->hdr.param1 = param1;
		this->hdr.param2 = param2;
		this->hdr.flags = flags;
		this->hdr.param3 = param3;
		this->hdr.size = data.size();
	}
	virtual ~NETDATA() {
	}


	bool compress(void) {
		if ((this->hdr.flags & NGWINAMP_FILTER_ALLOWZZIP) != 0) {
			if ((this->hdr.flags & NGWINAMP_FILTER_ZZIP) == 0) {
				dword size = this->buffer.size() + 32;
				byte  *tmp = new byte[size];
				int   code;

				this->buffer.read(tmp, 0, this->buffer.size());
				code = zzip_compress(tmp, this->buffer.size(), &size);
				if (code < 0) {
					delete [] tmp;
					return false;
				}
				if (size < this->buffer.size()) {
					this->hdr.flags |= NGWINAMP_FILTER_ZZIP;
					this->hdr.size = size;
					this->hdr.size2 = this->buffer.size();
					this->buffer.set(tmp, size);
				}
				delete [] tmp;
			}
			return true;
		}
		this->hdr.flags &= ~(NGWINAMP_FILTER_ZZIP | NGWINAMP_FILTER_ALLOWZZIP);
		this->hdr.size = this->buffer.size();
		this->hdr.size2 = 0;
		return false;
	}
	bool uncompress(void) {
		if ((this->hdr.flags & NGWINAMP_FILTER_ALLOWZZIP) != 0) {
			if ((this->hdr.flags & NGWINAMP_FILTER_ZZIP) != 0) {
				dword size = this->buffer.size() + 32;
				byte  *tmp = new byte[size];
				int   code;

				this->buffer.read(tmp, 0, this->buffer.size());
				code = zzip_decompress(tmp, this->buffer.size(), &size);
				if (code != 0) {
					delete [] tmp;
					return false;
				}
				this->buffer.set(tmp, size);
				delete [] tmp;

				this->hdr.flags &= ~NGWINAMP_FILTER_ZZIP;
				this->hdr.size = size;
				this->hdr.size2 = 0;
			}
			return true;
		}
		return false;
	}
};


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

#define NGWINAMP_SNAPSHOT_SETFLAG(pcon, flag)	(pcon)->snapshot.mflags |= (flag)
#define NGWINAMP_SNAPSHOT_HASFLAG(pcon, flag)	(((pcon)->snapshot.mflags & (flag)) != 0)
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


#endif //_NET_H_INCLUDE_
