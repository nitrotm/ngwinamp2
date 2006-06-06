// mainwnd.h
#ifndef _MAINWND_H_INCLUDE_
#define _MAINWND_H_INCLUDE_


#define CURSOR_MAX_COUNT	0x03
#define CURSOR_NORMAL		0x00
#define CURSOR_HRESIZE		0x01
#define CURSOR_VRESIZE		0x02

#define ICON_MAX_COUNT		0x01
#define ICON_MAIN			0x00

#define TOOLBOX_MAX_COUNT	0x04
#define TOOLBOX_CONNECT		0x00
#define TOOLBOX_CONNECTING	0x01
#define TOOLBOX_ADMIN		0x02
#define TOOLBOX_USER		0x03

#define IDC_STATUSBAR		1000

class NGMainWnd : public NGLOCK {
protected:
	// global
	HINSTANCE	hinstance;
	HCURSOR		cursors[CURSOR_MAX_COUNT];
	HICON		icons[ICON_MAX_COUNT];
	HIMAGELIST	himages;
	// mainwnd
	WNDCLASSEX	wndclass;
	ATOM		hclass;
	HWND		hwnd;
	HMENU		hmenu;
	int			x;
	int			y;
	dword		width;
	dword		height;
	dword		clientwidth;
	dword		clientheight;
	// statusbar
	HWND		hstatuswnd;
	dword		statuswidth;
	dword		statusheight;
	// toolbox
	HWND		htoolboxwnd[TOOLBOX_MAX_COUNT];
	dword		toolboxwidth[TOOLBOX_MAX_COUNT];
	dword		toolboxheight[TOOLBOX_MAX_COUNT];
	dword		toolboxminwidth;
	int			curtoolbox;
	// horizontal/vertical bar
	bool		hresize;
	bool		vresize;
	dword		lastmx;
	dword		lastmy;
	// browsewnd
	HWND		hbrowsewnd;
	dword		browsewidth;
	dword		browseheight;
	// filelistwnd
	HWND		hfilewnd;
	dword		filewidth;
	dword		fileheight;
	// playlistwnd
	HWND		hplwnd;
	dword		plwidth;
	dword		plheight;

	// network client
	NGWINAMPCLIENT *client;


public:
	NGMainWnd(HINSTANCE hinstance);
	virtual ~NGMainWnd();


	bool init(void);
	bool free(void);

	bool create(int x, int y, dword width, dword height);
	bool destroy(void);

	bool main(void);

	// window events
	bool onwnd_created(HWND hwnd);
	bool onwnd_close(void);
	bool onwnd_sizing(dword &width, dword &height);
	bool onwnd_resized(void);
	bool onwnd_mousemove(dword mx, dword my, bool mousedown);
	bool onwnd_command(WPARAM nc, WPARAM id);
	bool onwnd_notify(WPARAM id, LPNMHDR hdr);

	// network events
	bool onnet_connected(bool success, const string &title, const string &message);
	bool onnet_disconnect(void);
	bool onnet_authfailed(dword code);
	bool onnet_authsuccess(const NETAUTHEX &auth);
	bool onnet_setvolume(double volume);
	bool onnet_setpan(double pan);
	bool onnet_setposition(double progress, dword current, dword length);
	bool onnet_setshuffle(bool shuffle);
	bool onnet_setrepeat(bool repeat);


	void toolbox_select(int id);
};



#endif //_MAINWND_H_INCLUDE_
