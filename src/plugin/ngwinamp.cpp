// ngwinamp.cpp
#include "plugin.h"


NGWINAMP::NGWINAMP(HWND hwndplugin) : NGLOCK() {
	int version;

	// initialisation
	this->hplugin = hwndplugin;
	this->versionmajor = 0;
	this->versionminor = 0;

	// version de winamp
	version = SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)0, (LPARAM)IPC_GETVERSION);
	if (version > 0) {
		this->versionmajor = (version >> 8) & 0xFF;
		this->versionminor = version & 0xFF;
	}

	// chargement des fen�tres
	if (this->versionmajor >= 20) {
		this->hwinamp = FindWindow("Winamp v1.x", NULL);
		if(this->versionminor >= 90) {
			this->hplaylist = (HWND)SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)IPC_GETWND_PE, (LPARAM)IPC_GETWND);
		} else {
			this->hplaylist = FindWindow("Winamp PE", NULL);
		}
	}
}

NGWINAMP::~NGWINAMP() {
}

HWND NGWINAMP::getwinampwnd(void) const {
	return this->hwinamp;
}

HWND NGWINAMP::getplaylistwnd(void) const {
	return this->hplaylist;
}

word NGWINAMP::getmajorversion(void) const {
	return this->versionmajor;
}

word NGWINAMP::getminorversion(void) const {
	return this->versionminor;
}


// control
bool NGWINAMP::isplaying(void) {
	NGLOCKER lock(this);

	switch (SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)0, (LPARAM)IPC_ISPLAYING)) {
	case 1:
	case 3:
		return true;
	}
	return false;
}
bool NGWINAMP::ispaused(void) {
	NGLOCKER lock(this);

	if (SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)0, (LPARAM)IPC_ISPLAYING) == 3) {
		return true;
	}
	return false;
}
void NGWINAMP::prev(void) {
	NGLOCKER lock(this);

	if (this->pl_getlength() > 0) {
		SendMessage(this->hplugin, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON1, BN_CLICKED), (LPARAM)0);
	}
}
void NGWINAMP::play(void) {
	NGLOCKER lock(this);

	if (this->pl_getlength() > 0) {
		SendMessage(this->hplugin, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2, BN_CLICKED), (LPARAM)0);
	}
}
void NGWINAMP::pause(void) {
	NGLOCKER lock(this);

	if (this->pl_getlength() > 0) {
		SendMessage(this->hplugin, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON3, BN_CLICKED), (LPARAM)0);
	}
}
void NGWINAMP::stop(void) {
	NGLOCKER lock(this);

	if (this->pl_getlength() > 0) {
		SendMessage(this->hplugin, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON4, BN_CLICKED), (LPARAM)0);
	}
}
void NGWINAMP::next(void) {
	NGLOCKER lock(this);

	if (this->pl_getlength() > 0) {
		SendMessage(this->hplugin, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON5, BN_CLICKED), (LPARAM)0);
	}
}

// song
double NGWINAMP::sn_getvolume(void) {
	NGLOCKER lock(this);

	return double(SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)-666, (LPARAM)IPC_SETVOLUME)) / 255.0;
}
void NGWINAMP::sn_setvolume(double volume) {
	NGLOCKER lock(this);

	if (volume < 0.0) {
		volume = 0.0;
	}
	if (volume > 1.0) {
		volume = 1.0;
	}
	dword sn_volume = dword(volume * 255.0);

	SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)sn_volume, (LPARAM)IPC_SETVOLUME);
}
double NGWINAMP::sn_getpan(void) {
	NGLOCKER lock(this);

	return double(SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)-666, (LPARAM)IPC_SETPANNING)) / 127.0;
}
void NGWINAMP::sn_setpan(double pan) {
	NGLOCKER lock(this);

	if (pan < -1.0) {
		pan = -1.0;
	}
	if (pan > 1.0) {
		pan = 1.0;
	}
	long sn_pan = long(pan * 127.0);

	SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)sn_pan, (LPARAM)IPC_SETPANNING);
}
double NGWINAMP::sn_getpos(void) {
	NGLOCKER lock(this);

	if (this->isplaying()) {
		return (double(SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)0, (LPARAM)IPC_GETOUTPUTTIME)) / double(this->sn_getlength()));
	}
	return 0.0;
}
dword NGWINAMP::sn_getposms(void) {
	NGLOCKER lock(this);

	if (this->isplaying()) {
		return dword(SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)0, (LPARAM)IPC_GETOUTPUTTIME));
	}
	return 0;
}
void NGWINAMP::sn_setpos(double pos) {
	NGLOCKER lock(this);

	if (pos < 0.0) {
		pos = 0.0;
	}
	if (pos > 1.0) {
		pos = 1.0;
	}
	if (this->isplaying()) {
		dword sn_pos = dword(pos * this->sn_getlength());

		SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)sn_pos, (LPARAM)IPC_JUMPTOTIME);
	}
}
dword NGWINAMP::sn_getlength(void) {
	NGLOCKER lock(this);

	if (this->isplaying()) {
		return dword(SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)1, (LPARAM)IPC_GETOUTPUTTIME) * 1000);
	}
	return 0;
}

// playlist
void NGWINAMP::pl_clear(void) {
	NGLOCKER lock(this);

	SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)0, (LPARAM)IPC_DELETE);
}
dword NGWINAMP::pl_getlength(void) {
	NGLOCKER lock(this);

	return dword(SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)0, (LPARAM)IPC_GETLISTLENGTH));
}
void NGWINAMP::pl_setlength(dword length) {
	NGLOCKER lock(this);

	if (length > 0) {
		if (length < this->pl_getlength()) {
			for (dword i = this->pl_getlength() - 1; i > length; i--) {
				this->pl_delfile(i);
			}
		}
	} else {
		this->pl_clear();
	}
}
dword NGWINAMP::pl_getpos(void) {
	NGLOCKER lock(this);

	return dword(SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)0, (LPARAM)IPC_GETLISTPOS));
}
void NGWINAMP::pl_setpos(dword pos) {
	NGLOCKER lock(this);

	SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)pos, (LPARAM)IPC_SETPLAYLISTPOS);
}
bool NGWINAMP::pl_getshuffle(void) {
	NGLOCKER lock(this);

	if (SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)0, (LPARAM)IPC_GET_SHUFFLE) == 1) {
		return true;
	}
	return false;
}
void NGWINAMP::pl_setshuffle(bool shuffle) {
	NGLOCKER lock(this);

	if (shuffle) {
		SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)1, (LPARAM)IPC_SET_SHUFFLE);
	} else {
		SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)0, (LPARAM)IPC_SET_SHUFFLE);
	}
}
bool NGWINAMP::pl_getrepeat(void) {
	NGLOCKER lock(this);

	if (SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)0, (LPARAM)IPC_GET_REPEAT) == 1) {
		return true;
	}
	return false;
}
void NGWINAMP::pl_setrepeat(bool repeat) {
	NGLOCKER lock(this);

	if (repeat) {
		SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)1, (LPARAM)IPC_SET_REPEAT);
	} else {
		SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)0, (LPARAM)IPC_SET_REPEAT);
	}
}
bool NGWINAMP::pl_addfile(const string &filename) {
	NGLOCKER lock(this);

	// local directory ?
/*	if (this->isdirectory(filename)) {
		vector<string> items = this->getdirectoryitems(filename);

		for (dword i = 0; i < items.size(); i++) {
			this->pl_addfile(items[i]);
		}
		return true;
	}*/

	// remote or local file ?
	if (pathisurl(filename) || pathisfile(filename)) {
		COPYDATASTRUCT	cds;

		memset(&cds, 0, sizeof(COPYDATASTRUCT));
		cds.dwData = IPC_PLAYFILE;
		cds.lpData = (void*)filename.c_str();
		cds.cbData = filename.length() + 1;
		SendMessage(this->hplugin, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
		return true;
	}
	return false;
}
bool NGWINAMP::pl_insfile(dword index, const string &filename) {
	NGLOCKER lock(this);

	// local directory ?
/*	if (pathisdirectory(filename)) {
		vector<string> items = getdirectoryitems(filename);

		for (dword i = 0; i < items.size(); i++) {
			this->pl_insfile(index, items[i]);
		}
		return true;
	}
*/

	// remote or local file ?
	if (index >= 0 && index <= this->pl_getlength() && (pathisurl(filename) || pathisfile(filename))) {
		COPYDATASTRUCT	cds; 
		PE_FILEINFO		fi;

		memset(&fi, 0, sizeof(PE_FILEINFO));
		strcpy(fi.file, filename.c_str());
		fi.index = (int)index;

		memset(&cds, 0, sizeof(COPYDATASTRUCT));
		cds.dwData = IPC_PE_INSERTFILENAME; 
		cds.lpData = (void *)&fi; 
		cds.cbData = sizeof(PE_FILEINFO); 
		SendMessage(this->hplaylist, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds); 
		return true;
	}
	return false;
}
bool NGWINAMP::pl_setfile(dword index, const string &filename) {
	NGLOCKER lock(this);

	if (pathisurl(filename) || pathisfile(filename)) {
		if (this->pl_delfile(index) && this->pl_insfile(index, filename)) {
			return true;
		}
	}
	return false;
}
bool NGWINAMP::pl_delfile(dword index) {
	NGLOCKER lock(this);

	if (index >= 0 && index < this->pl_getlength()) {
		SendMessage(this->hplaylist, WM_WA_IPC, (WPARAM)IPC_PE_DELETEINDEX, (LPARAM)index); 
		return true;
	}
	return false;
}
bool NGWINAMP::pl_swapfile(dword index1, dword index2) {
	NGLOCKER lock(this);

	if (index1 >= 0 && index2 >= 0 && index1 < this->pl_getlength() && index2 < this->pl_getlength()) {
		SendMessage(this->hplaylist, WM_WA_IPC, (WPARAM)IPC_PE_SWAPINDEX, (LPARAM)index1);
		SendMessage(this->hplaylist, WM_WA_IPC, (WPARAM)IPC_PE_SWAPINDEX, (LPARAM)index2);
		SendMessage(this->hplaylist, WM_WA_IPC, (WPARAM)IPC_PE_SWAPINDEX, (LPARAM)index1);
		return true;
	}
	return false;
}
string NGWINAMP::pl_getfilename(dword index) {
	NGLOCKER lock(this);

	if (index >= 0 && index < this->pl_getlength()) {
		return string((char*)SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)index, (LPARAM)IPC_GETPLAYLISTFILE));
	}
	return string();
}
string NGWINAMP::pl_getname(dword index) {
	NGLOCKER lock(this);

	if (index >= 0 && index < this->pl_getlength()) {
		return string((char*)SendMessage(this->hplugin, WM_WA_IPC, (WPARAM)index, (LPARAM)IPC_GETPLAYLISTTITLE));
	}
	return string();
}
void NGWINAMP::pl_randomize(void) {
	NGLOCKER lock(this);

	SendMessage(this->hplaylist, WM_COMMAND, (WPARAM)MAKEWPARAM(IDC_NGWINAMP_PLRANDOMIZE, BN_CLICKED), (LPARAM)0);
}
void NGWINAMP::pl_sortbypath(void) {
	NGLOCKER lock(this);

	SendMessage(this->hplaylist, WM_COMMAND, (WPARAM)MAKEWPARAM(IDC_NGWINAMP_PLSORTBYPATH, BN_CLICKED), (LPARAM)0);
}
void NGWINAMP::pl_sortbyname(void) {
	NGLOCKER lock(this);

	SendMessage(this->hplaylist, WM_COMMAND, (WPARAM)MAKEWPARAM(IDC_NGWINAMP_PLSORTBYNAME, BN_CLICKED), (LPARAM)0);
}
void NGWINAMP::pl_removedeadfiles(void) {
	NGLOCKER lock(this);

	SendMessage(this->hplaylist, WM_COMMAND, (WPARAM)MAKEWPARAM(IDC_NGWINAMP_PLREMOVEDEAD, BN_CLICKED), (LPARAM)0);
}
