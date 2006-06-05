// mainwnd.h
#ifndef _MAINWND_H_INCLUDE_
#define _MAINWND_H_INCLUDE_


#define CURSOR_MAX_COUNT	0x02
#define CURSOR_NORMAL		0x00
#define CURSOR_HRESIZE		0x01

#define ICON_MAX_COUNT		0x01
#define ICON_MAIN			0x00

#define TOOLBOX_MAX_COUNT	0x04
#define TOOLBOX_CONNECT		0x00
#define TOOLBOX_CONNECTING	0x01
#define TOOLBOX_ADMIN		0x02
#define TOOLBOX_USER		0x03


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
	int			width;
	int			height;
	int			clientwidth;
	int			clientheight;
	// toolbox
	HWND		htoolboxwnd[TOOLBOX_MAX_COUNT];
	int			toolboxwidth[TOOLBOX_MAX_COUNT];
	int			toolboxheight[TOOLBOX_MAX_COUNT];
	int			toolboxminwidth;
	int			curtoolbox;
	// middlebar
	bool		hresize;
	int			lastmx;
	int			lastmy;
	// browsewnd
	HWND		hbrowsewnd;
	int			browsewidth;
	int			browseheight;
	// playlistwnd
	HWND		hplwnd;
	int			plwidth;
	int			plheight;

	// network client
	NGWINAMPCLIENT *client;


public:
	NGMainWnd(HINSTANCE hinstance);
	virtual ~NGMainWnd();


	bool init(void);
	bool free(void);

	bool create(int x, int y, int width, int height);
	bool destroy(void);

	bool main(void);

	// window events
	bool onwnd_created(HWND hwnd);
	bool onwnd_close(void);
	bool onwnd_resized(void);
	bool onwnd_mousemove(int mx, int my, bool mousedown);
	bool onwnd_command(WPARAM nc, WPARAM id);

	// network events
	bool onnet_connected(bool success, const string &title, const string &message);
	bool onnet_disconnect(void);
	bool onnet_authfailed(dword code);
	bool onnet_authsuccess(const NETAUTH &auth);
	bool onnet_authsuccess(const NETAUTHEX &auth);


	void toolbox_select(int id);
};



#endif //_MAINWND_H_INCLUDE_
