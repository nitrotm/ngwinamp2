#include "client.h"
#include "mainwnd.h"
#include "../../res/client/resource.h"


LRESULT CALLBACK mainwndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK toolboxdlgproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);



NGMainWnd::NGMainWnd(HINSTANCE hinstance) : hinstance(hinstance), hclass(0), hwnd(NULL), hplwnd(NULL), hbrowsewnd(NULL), himages(NULL) {
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


bool NGMainWnd::init() {
	// load cursors
	this->cursors[CURSOR_NORMAL] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	this->cursors[CURSOR_HRESIZE] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZEWE), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);

	// load icons
	this->icons[ICON_MAIN] = (HICON)LoadImage(this->hinstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED);

	// build imagelist
	this->himages = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 4);
	ImageList_AddIcon(this->himages, this->icons[ICON_MAIN]);

	// load menus
	this->hmenu = LoadMenu(this->hinstance, MAKEINTRESOURCE(IDR_MAIN));

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
		return false;
	}

	this->curtoolbox = TOOLBOX_CONNECT;
	return true;
}
bool NGMainWnd::free() {
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
	// create main window
	RECT rc;

	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->hwnd = CreateWindowEx(0, (LPCTSTR)this->hclass, NGWINAMP_NAME, WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW, this->x, this->y, this->width, this->height, NULL, this->hmenu, this->hinstance, this);
	if (this->hwnd == NULL) {
		return false;
	}
	GetClientRect(this->hwnd, &rc);
	this->clientwidth = rc.right - rc.left;
	this->clientheight = rc.bottom - rc.top;

	// create toolbox window
	this->htoolboxwnd[TOOLBOX_CONNECT] = CreateDialog(this->hinstance, MAKEINTRESOURCE(IDD_TOOLBOX_CONNECT), this->hwnd, (DLGPROC)toolboxdlgproc);
	if (this->htoolboxwnd[TOOLBOX_CONNECT] == NULL) {
		return false;
	}
	GetWindowRect(this->htoolboxwnd[TOOLBOX_CONNECT], &rc);
	this->toolboxwidth[TOOLBOX_CONNECT] = this->clientwidth;
	this->toolboxheight[TOOLBOX_CONNECT] = rc.bottom - rc.top;
	SetWindowPos(this->htoolboxwnd[TOOLBOX_CONNECT], NULL, 0, 0, this->toolboxwidth[TOOLBOX_CONNECT], this->toolboxheight[TOOLBOX_CONNECT], SWP_NOZORDER);

	// create browse window
	this->browsewidth = this->clientwidth / NGWINAMP_BROWSER_RATIO;
	this->browseheight = this->clientheight - this->toolboxheight[this->curtoolbox];
	this->hbrowsewnd = CreateWindowEx(WS_EX_STATICEDGE, WC_TREEVIEW, "BrowserWindow", WS_CHILD | WS_VISIBLE, 0, this->toolboxheight[this->curtoolbox], this->browsewidth, this->browseheight, this->hwnd, NULL, this->hinstance, NULL);
	if (this->hbrowsewnd == NULL) {
		return false;
	}

	// create playlist window
	this->plwidth = this->clientwidth - this->browsewidth - NGWINAMP_BAR_WIDTH;
	this->plheight = this->clientheight - this->toolboxheight[this->curtoolbox];
	this->hplwnd = CreateWindowEx(WS_EX_STATICEDGE,
		WC_LISTVIEW, "PlaylistWindow", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox], this->plwidth, this->plheight, this->hwnd, NULL, this->hinstance, NULL);
	if (this->hplwnd == NULL) {
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
bool NGMainWnd::destroy() {
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


bool NGMainWnd::main() {
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


bool NGMainWnd::on_created(HWND hwnd) {
	this->hwnd = hwnd;
	ShowWindow(this->hwnd, SW_SHOW);
	return true;
}

bool NGMainWnd::on_close() {
	ShowWindow(this->hwnd, SW_HIDE);
	PostQuitMessage(0);
	return true;
}


bool NGMainWnd::on_resized(int width, int height) {
	RECT rc;

	GetClientRect(this->hwnd, &rc);
	this->clientwidth = rc.right - rc.left;
	this->clientheight = rc.bottom - rc.top;

	this->toolboxwidth[this->curtoolbox] = this->clientwidth;
	SetWindowPos(this->htoolboxwnd[this->curtoolbox], NULL, 0, 0, this->toolboxwidth[this->curtoolbox], this->toolboxheight[this->curtoolbox], SWP_NOZORDER);

	GetClientRect(this->htoolboxwnd[this->curtoolbox], &rc);
	SetWindowPos(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_TOOLBOX_TITLE), NULL, rc.left + NGWINAMP_TOOLBOX_TITLE_MARGIN + 1, rc.top + NGWINAMP_TOOLBOX_TITLE_MARGIN, rc.right - rc.left - 2 * NGWINAMP_TOOLBOX_TITLE_MARGIN, rc.bottom - rc.top - 2 * NGWINAMP_TOOLBOX_TITLE_MARGIN, SWP_NOZORDER);

	if (this->browsewidth < (NGWINAMP_BAR_WIDTH + NGWINAMP_MIN_PANEL_WIDTH)) {
		this->browsewidth = NGWINAMP_BAR_WIDTH + NGWINAMP_MIN_PANEL_WIDTH;
	}
	if (this->browsewidth > (this->clientwidth - NGWINAMP_BAR_WIDTH - NGWINAMP_MIN_PANEL_WIDTH)) {
		this->browsewidth = this->clientwidth - NGWINAMP_BAR_WIDTH - NGWINAMP_MIN_PANEL_WIDTH;
	}
	this->browseheight = this->clientheight - this->toolboxheight[this->curtoolbox];
	SetWindowPos(this->hbrowsewnd, NULL, 0, this->toolboxheight[this->curtoolbox], this->browsewidth, this->browseheight, SWP_NOZORDER);

	this->plwidth = this->clientwidth - this->browsewidth - NGWINAMP_BAR_WIDTH;
	this->plheight = this->clientheight - this->toolboxheight[this->curtoolbox];
	SetWindowPos(this->hplwnd, NULL, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox], this->plwidth, this->plheight, SWP_NOZORDER);
	return true;
}


bool NGMainWnd::on_mousemove(int mx, int my, bool mousedown) {
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
					SetWindowPos(this->hbrowsewnd, NULL, 0, this->toolboxheight[this->curtoolbox], this->browsewidth, this->browseheight, SWP_NOZORDER);

					this->plwidth = this->clientwidth - this->browsewidth - NGWINAMP_BAR_WIDTH;
					SetWindowPos(this->hplwnd, NULL, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox], this->plwidth, this->plheight, SWP_NOZORDER);
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
			if (!pmainwnd->on_created(hwnd)) {
				PostQuitMessage(0);
			}
			return 0;

		case WM_SIZE:
			if (wparam != SIZE_MINIMIZED) {
				if (!pmainwnd->on_resized(LOWORD(lparam), HIWORD(lparam))) {
					PostQuitMessage(0);
				}
			}
			return 0;

		case WM_CLOSE:
			if (!pmainwnd->on_close()) {
				PostQuitMessage(0);
			}
			return 0;

		case WM_MOUSEMOVE:
			if (!pmainwnd->on_mousemove(LOWORD(lparam), HIWORD(lparam), wparam & MK_LBUTTON)) {
				PostQuitMessage(0);
			}
			break;
		}
	}
	return DefWindowProc(hwnd, message, wparam, lparam);
}

BOOL CALLBACK toolboxdlgproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
	case WM_INITDIALOG:
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
