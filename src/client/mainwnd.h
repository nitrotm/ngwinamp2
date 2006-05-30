#ifndef _MAINWND_H_INCLUDE_
#define _MAINWND_H_INCLUDE_


#define CURSOR_MAX_COUNT	0x02
#define CURSOR_NORMAL		0x00
#define CURSOR_HRESIZE		0x01

#define ICON_MAX_COUNT		0x01
#define ICON_MAIN			0x00

#define TOOLBOX_MAX_COUNT	0x04
#define TOOLBOX_CONNECT		0x00
#define TOOLBOX_ADMIN		0x01
#define TOOLBOX_USER		0x02
#define TOOLBOX_GUEST		0x03


class NGMainWnd {
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


public:
	NGMainWnd(HINSTANCE hinstance);
	virtual ~NGMainWnd();


	bool init();
	bool free();

	bool create(int x, int y, int width, int height);
	bool destroy();

	bool main();

	bool on_created(HWND hwnd);
	bool on_close();

	bool on_resized(int width, int height);

	bool on_mousemove(int mx, int my, bool mousedown);
};



#endif //_MAINWND_H_INCLUDE_
