// ngwinamp.h
#ifndef _NGWINAMP_H_INCLUDE_
#define _NGWINAMP_H_INCLUDE_


/**
  * Winamp control class
  *
  */
class NGWINAMP : public NGLOCK {
private:
	// infos
	word	versionmajor;
	word	versionminor;

	// gui
	HWND	hwinamp;
	HWND	hplaylist;
	HWND	hplugin;
/*
	// current data
	dword			au_volume;
	dword			au_pan;
	dword			pl_position;
	dword			pl_length;
	bool			pl_shuffle;
	bool			pl_repeat;

	// current song
	dword			sn_length;
	dword			sn_pos;
*/

public:
	// constructor / destructor
	NGWINAMP(HWND hwndplugin);
	virtual ~NGWINAMP();


	HWND   getwinampwnd(void) const;
	HWND   getplaylistwnd(void) const;
	word   getmajorversion(void) const;
	word   getminorversion(void) const;

	// basic controls
	bool   isplaying(void);
	bool   ispaused(void);
	void   prev(void);
	void   play(void);
	void   pause(void);
	void   stop(void);
	void   next(void);

	// sound controls
	double sn_getvolume(void);
	void   sn_setvolume(double volume);
	double sn_getpan(void);
	void   sn_setpan(double pan);
	double sn_getpos(void);
	dword  sn_getposms(void);
	void   sn_setpos(double pos);
	dword  sn_getlength(void);

	// playlist controls
	void   pl_clear(void);
	dword  pl_getlength(void);
	void   pl_setlength(dword length);
	dword  pl_getpos(void);
	void   pl_setpos(dword pos);
	bool   pl_getshuffle(void);
	void   pl_setshuffle(bool shuffle);
	bool   pl_getrepeat(void);
	void   pl_setrepeat(bool repeat);
	bool   pl_addfile(const vector<string> &filename);
	bool   pl_insfile(dword index, const vector<string> &filename);
	bool   pl_setfile(dword index, const vector<string> &filename);
	bool   pl_delfile(const vector<dword> &index);
	bool   pl_swapfile(dword index1, dword index2);
	string pl_getfilename(dword index);
	string pl_getname(dword index);
	void   pl_randomize(void);
	void   pl_sortbypath(void);
	void   pl_sortbyname(void);
	void   pl_removedeadfiles(void);
};


#endif //_NGWINAMP_H_INCLUDE_
