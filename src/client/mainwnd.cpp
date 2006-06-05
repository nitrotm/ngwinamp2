// mainwnd.cpp
#include "client.h"
#include "../net.h"
#include "../netaddr.h"
#include "../netdata.h"
#include "ngwinampclient.h"
#include "mainwnd.h"
#include "../../res/client/resource.h"


LRESULT CALLBACK mainwndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK toolboxdlgproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);



NGMainWnd::NGMainWnd(HINSTANCE hinstance) : NGLOCK(), hinstance(hinstance), hclass(0), hwnd(NULL), hplwnd(NULL), hbrowsewnd(NULL), himages(NULL), client(NULL) {
	for (int i = 0; i < CURSOR_MAX_COUNT; i++) {
		this->cursors[i] = NULL;
	}
	for (int i = 0; i < ICON_MAX_COUNT; i++) {
		this->icons[i] = NULL;
	}
	for (int i = 0; i < TOOLBOX_MAX_COUNT; i++) {
		this->htoolboxwnd[i] = NULL;
		this->toolboxwidth[i] = 0;
		this->toolboxheight[i] = 0;
	}
}
NGMainWnd::~NGMainWnd() {
	this->destroy();
	this->free();
}


bool NGMainWnd::init(void) {
	NGLOCKER locker(this);

	// load cursors
	this->cursors[CURSOR_NORMAL] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	if (this->cursors[CURSOR_NORMAL] == NULL) {
		DEBUGWRITE("NGMainWnd::init() cannot load normal cursor !");
		return false;
	}
	this->cursors[CURSOR_HRESIZE] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZEWE), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	if (this->cursors[CURSOR_HRESIZE] == NULL) {
		DEBUGWRITE("NGMainWnd::init() cannot load hresize cursor !");
		return false;
	}

	// load icons
	this->icons[ICON_MAIN] = (HICON)LoadImage(this->hinstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED);
	if (this->icons[ICON_MAIN] == NULL) {
		DEBUGWRITE("NGMainWnd::init() cannot load main icon !");
		return false;
	}

	// build imagelist
	this->himages = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 4);
	if (this->himages == NULL) {
		DEBUGWRITE("NGMainWnd::init() cannot create image list !");
		return false;
	}
	ImageList_AddIcon(this->himages, this->icons[ICON_MAIN]);

	// load menus
	this->hmenu = LoadMenu(this->hinstance, MAKEINTRESOURCE(IDR_MAIN));
	if (this->hmenu == NULL) {
		DEBUGWRITE("NGMainWnd::init() cannot load main menu !");
		return false;
	}

	// setup main class
	memset(&this->wndclass, 0, sizeof(WNDCLASSEX));
	this->wndclass.cbSize = sizeof(WNDCLASSEX);
	this->wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	this->wndclass.lpfnWndProc = (WNDPROC)mainwndproc;
	this->wndclass.cbClsExtra = 0;
	this->wndclass.cbWndExtra = 0;
	this->wndclass.hInstance = this->hinstance;
	this->wndclass.hIcon = this->icons[ICON_MAIN];
	this->wndclass.hCursor = this->cursors[CURSOR_NORMAL];
	this->wndclass.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE;
	this->wndclass.lpszMenuName = NULL;
	this->wndclass.lpszClassName = "NGWINAMP2_MAINWND_CLASS";
	this->wndclass.hIconSm = NULL;

	// register main class
	this->hclass = RegisterClassEx(&this->wndclass);
	if (this->hclass == 0) {
		DEBUGWRITE("NGMainWnd::init() cannot register main window class !");
		return false;
	}

	// create client instance
	this->client = new NGWINAMPCLIENT(this);
	if (this->client == NULL) {
		DEBUGWRITE("NGMainWnd::init() cannot create network client !");
		return false;
	}

	this->curtoolbox = -1;
	return true;
}
bool NGMainWnd::free(void) {
	NGLOCKER locker(this);

	// free client instance
	if (this->client != NULL) {
		locker.release();
		delete [] this->client;
		locker.acquire();
		this->client = NULL;
	}

	// free main class
	if (this->hclass != 0) {
		UnregisterClass((LPCTSTR)this->hclass, this->hinstance);
	}

	// free menu
	if (this->hmenu != NULL) {
		DestroyMenu(this->hmenu);
		this->hmenu = NULL;
	}

	// free imagelist
	if (this->himages != NULL) {
		ImageList_Destroy(this->himages);
		this->himages = NULL;
	}

	// free icons
	for (int i = 0; i < ICON_MAX_COUNT; i++) {
		if (this->icons[i] != NULL) {
			DestroyIcon(this->icons[i]);
			this->icons[i] = NULL;
		}
	}

	// free cursors
	for (int i = 0; i < CURSOR_MAX_COUNT; i++) {
		if (this->cursors[i] != NULL) {
			DestroyCursor(this->cursors[i]);
			this->cursors[i] = NULL;
		}
	}
	return true;
}

bool NGMainWnd::create(int x, int y, int width, int height) {
	NGLOCKER locker(this);

	// create main window
	RECT rc;

	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->hwnd = CreateWindowEx(0, (LPCTSTR)this->hclass, NGWINAMP_NAME, WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW, this->x, this->y, this->width, this->height, NULL, this->hmenu, this->hinstance, this);
	if (this->hwnd == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create main window !");
		return false;
	}
	GetClientRect(this->hwnd, &rc);
	this->clientwidth = rc.right - rc.left;
	this->clientheight = rc.bottom - rc.top;
	this->toolboxminwidth = 0;

	// create connect toolbox window
	this->htoolboxwnd[TOOLBOX_CONNECT] = CreateDialog(this->hinstance, MAKEINTRESOURCE(IDD_TOOLBOX_CONNECT), this->hwnd, (DLGPROC)toolboxdlgproc);
	if (this->htoolboxwnd[TOOLBOX_CONNECT] == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create connect toolbox window !");
		return false;
	}
	GetWindowRect(this->htoolboxwnd[TOOLBOX_CONNECT], &rc);
	this->toolboxwidth[TOOLBOX_CONNECT] = this->clientwidth;
	this->toolboxheight[TOOLBOX_CONNECT] = rc.bottom - rc.top;
	if (this->toolboxminwidth < this->toolboxwidth[TOOLBOX_CONNECT]) {
		this->toolboxminwidth = this->toolboxwidth[TOOLBOX_CONNECT];
	}
	SetWindowPos(this->htoolboxwnd[TOOLBOX_CONNECT], NULL, 0, 0, this->toolboxwidth[TOOLBOX_CONNECT], this->toolboxheight[TOOLBOX_CONNECT], SWP_NOZORDER);

	// create connecting toolbox window
	this->htoolboxwnd[TOOLBOX_CONNECTING] = CreateDialog(this->hinstance, MAKEINTRESOURCE(IDD_TOOLBOX_CONNECTING), this->hwnd, (DLGPROC)toolboxdlgproc);
	if (this->htoolboxwnd[TOOLBOX_CONNECTING] == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create connecting toolbox window !");
		return false;
	}
	GetWindowRect(this->htoolboxwnd[TOOLBOX_CONNECTING], &rc);
	this->toolboxwidth[TOOLBOX_CONNECTING] = this->clientwidth;
	this->toolboxheight[TOOLBOX_CONNECTING] = rc.bottom - rc.top;
	if (this->toolboxminwidth < this->toolboxwidth[TOOLBOX_CONNECTING]) {
		this->toolboxminwidth = this->toolboxwidth[TOOLBOX_CONNECTING];
	}
	SetWindowPos(this->htoolboxwnd[TOOLBOX_CONNECTING], NULL, 0, 0, this->toolboxwidth[TOOLBOX_CONNECTING], this->toolboxheight[TOOLBOX_CONNECTING], SWP_NOZORDER);

	// create admin toolbox window
	this->htoolboxwnd[TOOLBOX_ADMIN] = CreateDialog(this->hinstance, MAKEINTRESOURCE(IDD_TOOLBOX_ADMIN), this->hwnd, (DLGPROC)toolboxdlgproc);
	if (this->htoolboxwnd[TOOLBOX_ADMIN] == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create admin toolbox window !");
		return false;
	}
	GetWindowRect(this->htoolboxwnd[TOOLBOX_ADMIN], &rc);
	this->toolboxwidth[TOOLBOX_ADMIN] = this->clientwidth;
	this->toolboxheight[TOOLBOX_ADMIN] = rc.bottom - rc.top;
	if (this->toolboxminwidth < this->toolboxwidth[TOOLBOX_ADMIN]) {
		this->toolboxminwidth = this->toolboxwidth[TOOLBOX_ADMIN];
	}
	SetWindowPos(this->htoolboxwnd[TOOLBOX_ADMIN], NULL, 0, 0, this->toolboxwidth[TOOLBOX_ADMIN], this->toolboxheight[TOOLBOX_ADMIN], SWP_NOZORDER);

	// create user toolbox window
	this->htoolboxwnd[TOOLBOX_USER] = CreateDialog(this->hinstance, MAKEINTRESOURCE(IDD_TOOLBOX_USER), this->hwnd, (DLGPROC)toolboxdlgproc);
	if (this->htoolboxwnd[TOOLBOX_USER] == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create user toolbox window !");
		return false;
	}
	GetWindowRect(this->htoolboxwnd[TOOLBOX_USER], &rc);
	this->toolboxwidth[TOOLBOX_USER] = this->clientwidth;
	this->toolboxheight[TOOLBOX_USER] = rc.bottom - rc.top;
	if (this->toolboxminwidth < this->toolboxwidth[TOOLBOX_USER]) {
		this->toolboxminwidth = this->toolboxwidth[TOOLBOX_USER];
	}
	SetWindowPos(this->htoolboxwnd[TOOLBOX_USER], NULL, 0, 0, this->toolboxwidth[TOOLBOX_USER], this->toolboxheight[TOOLBOX_USER], SWP_NOZORDER);

	// select initial toolbox (connect)
	this->toolbox_select(TOOLBOX_CONNECT);

	// create browse window
	this->browsewidth = this->clientwidth / NGWINAMP_BROWSER_RATIO;
	this->browseheight = this->clientheight - this->toolboxheight[this->curtoolbox];
	this->hbrowsewnd = CreateWindowEx(WS_EX_STATICEDGE, WC_TREEVIEW, "BrowserWindow", WS_CHILD | WS_VISIBLE, 0, this->toolboxheight[this->curtoolbox], this->browsewidth, this->browseheight, this->hwnd, NULL, this->hinstance, NULL);
	if (this->hbrowsewnd == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create browse window !");
		return false;
	}

	// create playlist window
	this->plwidth = this->clientwidth - this->browsewidth - NGWINAMP_BAR_WIDTH;
	this->plheight = this->clientheight - this->toolboxheight[this->curtoolbox];
	this->hplwnd = CreateWindowEx(WS_EX_STATICEDGE,
		WC_LISTVIEW, "PlaylistWindow", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox], this->plwidth, this->plheight, this->hwnd, NULL, this->hinstance, NULL);
	if (this->hplwnd == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create playlist window !");
		return false;
	}
	SendMessage(this->hplwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_FLATSB, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_FLATSB);

	// setup initial toolbox
	GetClientRect(this->htoolboxwnd[this->curtoolbox], &rc);
	SetWindowPos(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_TOOLBOX_TITLE), NULL, rc.left + NGWINAMP_TOOLBOX_TITLE_MARGIN + 1, rc.top + NGWINAMP_TOOLBOX_TITLE_MARGIN, rc.right - rc.left - 2 * NGWINAMP_TOOLBOX_TITLE_MARGIN, rc.bottom - rc.top - 2 * NGWINAMP_TOOLBOX_TITLE_MARGIN, SWP_NOZORDER);

	// setup browse window
	EnableWindow(this->hbrowsewnd, FALSE);

	// setup playlist window
	LVCOLUMN lvc;

	memset(&lvc, 0, sizeof(LVCOLUMN));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.iSubItem = 0;
	lvc.cx = 300;
	lvc.pszText = NGWINAMP_PL_HEADER_TITLE;
	ListView_InsertColumn(this->hplwnd, 0, &lvc);
	lvc.iSubItem = 1;
	lvc.cx = 300;
	lvc.pszText = NGWINAMP_PL_HEADER_PATH;
	ListView_InsertColumn(this->hplwnd, 1, &lvc);
	EnableWindow(this->hplwnd, FALSE);
	return true;
}
bool NGMainWnd::destroy(void) {
	NGLOCKER locker(this);

	for (int i = 0; i < TOOLBOX_MAX_COUNT; i++) {
		if (this->htoolboxwnd[i] != NULL) {
			DestroyWindow(this->htoolboxwnd[i]);
			this->htoolboxwnd[i] = NULL;
		}
		this->toolboxwidth[i] = 0;
		this->toolboxheight[i] = 0;
	}
	if (this->hbrowsewnd != NULL) {
		DestroyWindow(this->hbrowsewnd);
		this->hbrowsewnd = NULL;
	}
	if (this->hplwnd != NULL) {
		DestroyWindow(this->hplwnd);
		this->hplwnd = NULL;
	}
	if (this->hwnd != NULL) {
		DestroyWindow(this->hwnd);
		this->hwnd = NULL;
	}
	return true;
}


bool NGMainWnd::main(void) {
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) {
		if (this->htoolboxwnd[this->curtoolbox] != NULL && IsDialogMessage(this->htoolboxwnd[this->curtoolbox], &msg)) {
			continue;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}



bool NGMainWnd::onwnd_created(HWND hwnd) {
	this->hwnd = hwnd;
	ShowWindowAsync(this->hwnd, SW_SHOW);
	return true;
}

bool NGMainWnd::onwnd_close(void) {
	ShowWindowAsync(this->hwnd, SW_HIDE);
	PostQuitMessage(0);
	return true;
}

bool NGMainWnd::onwnd_resized(void) {
	RECT rc;

	GetClientRect(this->hwnd, &rc);
	this->clientwidth = rc.right - rc.left;
	this->clientheight = rc.bottom - rc.top;

	this->toolboxwidth[this->curtoolbox] = this->clientwidth;
	SetWindowPos(this->htoolboxwnd[this->curtoolbox], NULL, 0, 0, this->toolboxwidth[this->curtoolbox], this->toolboxheight[this->curtoolbox], SWP_NOZORDER | SWP_ASYNCWINDOWPOS);

	GetClientRect(this->htoolboxwnd[this->curtoolbox], &rc);
	SetWindowPos(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_TOOLBOX_TITLE), NULL, rc.left + NGWINAMP_TOOLBOX_TITLE_MARGIN + 1, rc.top + NGWINAMP_TOOLBOX_TITLE_MARGIN, rc.right - rc.left - 2 * NGWINAMP_TOOLBOX_TITLE_MARGIN, rc.bottom - rc.top - 2 * NGWINAMP_TOOLBOX_TITLE_MARGIN, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);

	if (this->browsewidth < (NGWINAMP_BAR_WIDTH + NGWINAMP_MIN_PANEL_WIDTH)) {
		this->browsewidth = NGWINAMP_BAR_WIDTH + NGWINAMP_MIN_PANEL_WIDTH;
	}
	if (this->browsewidth > (this->clientwidth - NGWINAMP_BAR_WIDTH - NGWINAMP_MIN_PANEL_WIDTH)) {
		this->browsewidth = this->clientwidth - NGWINAMP_BAR_WIDTH - NGWINAMP_MIN_PANEL_WIDTH;
	}
	this->browseheight = this->clientheight - this->toolboxheight[this->curtoolbox];
	SetWindowPos(this->hbrowsewnd, NULL, 0, this->toolboxheight[this->curtoolbox], this->browsewidth, this->browseheight, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);

	this->plwidth = this->clientwidth - this->browsewidth - NGWINAMP_BAR_WIDTH;
	this->plheight = this->clientheight - this->toolboxheight[this->curtoolbox];
	SetWindowPos(this->hplwnd, NULL, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox], this->plwidth, this->plheight, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);
	return true;
}

bool NGMainWnd::onwnd_mousemove(int mx, int my, bool mousedown) {
	if (this->hresize) {
		if (mousedown) {
			if (mx >= 0 && mx < this->clientwidth) {
				int delta = mx - this->lastmx;

				this->lastmx = mx;
				this->lastmy = my;

				if (delta != 0) {
					this->browsewidth += delta;
					if (this->browsewidth > (this->clientwidth - NGWINAMP_BAR_WIDTH - NGWINAMP_MIN_PANEL_WIDTH)) {
						this->browsewidth = this->clientwidth - NGWINAMP_BAR_WIDTH - NGWINAMP_MIN_PANEL_WIDTH;
					}
					if (this->browsewidth < (NGWINAMP_BAR_WIDTH + NGWINAMP_MIN_PANEL_WIDTH)) {
						this->browsewidth = NGWINAMP_BAR_WIDTH + NGWINAMP_MIN_PANEL_WIDTH;
					}
					SetWindowPos(this->hbrowsewnd, NULL, 0, this->toolboxheight[this->curtoolbox], this->browsewidth, this->browseheight, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);

					this->plwidth = this->clientwidth - this->browsewidth - NGWINAMP_BAR_WIDTH;
					SetWindowPos(this->hplwnd, NULL, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox], this->plwidth, this->plheight, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);
				}
			}
		} else {
			this->hresize = false;
			ReleaseCapture();
		}
	}
	if (mx >= this->browsewidth && mx < (this->browsewidth + NGWINAMP_BAR_WIDTH) &&
		my >= 0 && my < this->clientheight) {
		SetCursor(this->cursors[CURSOR_HRESIZE]);
		if (mousedown && !this->hresize) {
			this->hresize = true;
			this->lastmx = mx;
			this->lastmy = my;
			SetCapture(this->hwnd);
		}
	} else {
		if (!this->hresize) {
			SetCursor(this->cursors[CURSOR_NORMAL]);
		}
	}
	return true;
}

bool NGMainWnd::onwnd_command(WPARAM nc, WPARAM id) {
	if (nc == BN_CLICKED) {
		switch (id) {
		case IDM_DISCONNECT:
			if (this->client->isrunning()) {
				if (!this->client->stop()) {
					DEBUGWRITE("NGMainWnd::onwnd_command() error when disconnecting from server !");
					return false;
				}
			}
			return true;

		case IDM_EXIT:
			return this->onwnd_close();
		}
		switch (this->curtoolbox) {
		case TOOLBOX_CONNECT:
			switch (id) {
			case IDM_CONNECT:
			case IDC_CONNECT:
				if (!this->client->isrunning()) {
					if (!this->client->start(textbox_getstring(this->htoolboxwnd[TOOLBOX_CONNECT], IDC_HOSTNAME),
						GetDlgItemInt(this->htoolboxwnd[TOOLBOX_CONNECT], IDC_PORT, NULL, FALSE))) {
						DEBUGWRITE("NGMainWnd::onwnd_command() error when connecting to server !");
						return false;
					}
					this->toolbox_select(TOOLBOX_CONNECTING);
					SetDlgItemText(this->htoolboxwnd[TOOLBOX_CONNECTING], IDC_MESSAGE, "connecting to server...");
				}
				return true;
			}
			break;
		}
	}
	return true;
}



bool NGMainWnd::onnet_connected(bool success, const string &title, const string &message) {
	if (success) {
		SetDlgItemText(this->htoolboxwnd[TOOLBOX_CONNECTING], IDC_MESSAGE, "request authentication...");
		return this->client->authenticate(textbox_getstring(this->htoolboxwnd[TOOLBOX_CONNECT], IDC_USERNAME), textbox_getstring(this->htoolboxwnd[TOOLBOX_CONNECT], IDC_PASSWORD));
	}
	MessageBox(this->hwnd, message.c_str(), title.c_str(), MB_ICONERROR);
	return false;
}

bool NGMainWnd::onnet_disconnect(void) {
	this->toolbox_select(TOOLBOX_CONNECT);
	return true;
}

bool NGMainWnd::onnet_authfailed(dword code) {
	switch (code) {
	case NGWINAMP_AUTH_FAILURE:
		MessageBox(this->hwnd, "Invalid username/password !", "Authentication error", MB_ICONERROR);
		break;
	case NGWINAMP_AUTH_NOTDONE:
		MessageBox(this->hwnd, "Authentication not done !", "Authentication error", MB_ICONERROR);
		break;
	case NGWINAMP_AUTH_TOOMANYCON:
		MessageBox(this->hwnd, "Too many connections !", "Authentication error", MB_ICONERROR);
		break;
	}
	return false;
}
bool NGMainWnd::onnet_authsuccess(const NETAUTH &auth) {
	this->toolbox_select(TOOLBOX_USER);
	return true;
}
bool NGMainWnd::onnet_authsuccess(const NETAUTHEX &auth) {
	this->toolbox_select(TOOLBOX_ADMIN);
	return true;
}



void NGMainWnd::toolbox_select(int id) {
	if (this->curtoolbox != id) {
		this->curtoolbox = id;
		for (int i = 0; i < TOOLBOX_MAX_COUNT; i++) {
			if (i != id && this->htoolboxwnd[i] != NULL) {
				ShowWindowAsync(this->htoolboxwnd[i], SW_HIDE);
			}
		}
		ShowWindowAsync(this->htoolboxwnd[id], SW_SHOW);
		this->onwnd_resized();
	}
}




LRESULT CALLBACK mainwndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	NGMainWnd *pmainwnd;

	if (message == WM_CREATE) {
		CREATESTRUCT *cs = (CREATESTRUCT*)lparam;

		pmainwnd = (NGMainWnd*)cs->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pmainwnd);
	} else {
		pmainwnd = (NGMainWnd*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}
	if (pmainwnd != NULL) {
		switch (message) {
		case WM_CREATE:
			if (!pmainwnd->onwnd_created(hwnd)) {
				DEBUGWRITE("mainwndproc() error processing WM_CREATE event !");
				PostQuitMessage(0);
			}
			return 0;

		case WM_SIZE:
			if (wparam != SIZE_MINIMIZED) {
				if (!pmainwnd->onwnd_resized()) {
					DEBUGWRITE("mainwndproc() error processing WM_SIZE event !");
					PostQuitMessage(0);
				}
			}
			return 0;

		case WM_MOUSEMOVE:
			if (!pmainwnd->onwnd_mousemove(LOWORD(lparam), HIWORD(lparam), wparam & MK_LBUTTON)) {
				DEBUGWRITE("mainwndproc() error processing WM_MOUSEMOVE event !");
				PostQuitMessage(0);
			}
			return 0;

		case WM_COMMAND:
			if (!pmainwnd->onwnd_command(HIWORD(wparam), LOWORD(wparam))) {
				DEBUGWRITE("mainwndproc() error processing WM_COMMAND event !");
				PostQuitMessage(0);
			}
			return 0;

		case WM_CLOSE:
			if (!pmainwnd->onwnd_close()) {
				DEBUGWRITE("mainwndproc() error processing WM_CLOSE event !");
				PostQuitMessage(0);
			}
			return 0;
		}
	}
	return DefWindowProc(hwnd, message, wparam, lparam);
}

BOOL CALLBACK toolboxdlgproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	NGMainWnd *pmainwnd = (NGMainWnd*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);

	switch (message) {
	case WM_INITDIALOG:
		break;

	case WM_COMMAND:
		if (!pmainwnd->onwnd_command(HIWORD(wparam), LOWORD(wparam))) {
			DEBUGWRITE("toolboxdlgproc() error processing WM_COMMAND event (control) !");
			PostQuitMessage(0);
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
