// mainwnd.cpp
#include "../global.h"
#include "../util.h"
#include "../net.h"
#include "../netaddr.h"
#include "../netdata.h"
#include "../netauth.h"
#include "client.h"
#include "ngwinampclient.h"
#include "mainwnd.h"

#include "../../res/client/resource.h"


LRESULT CALLBACK mainwndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK toolboxdlgproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);



NGMainWnd::NGMainWnd(HINSTANCE hinstance) : NGLOCK(), hinstance(hinstance), hclass(0), hwnd(NULL), hplwnd(NULL), hbrowsewnd(NULL), himages(NULL), client(NULL), playlist_pos(-1), playlist_size(0), browseindex(-1), dragndrop(NULL) {
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
	this->cursors[CURSOR_VRESIZE] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZENS), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	if (this->cursors[CURSOR_VRESIZE] == NULL) {
		DEBUGWRITE("NGMainWnd::init() cannot load vresize cursor !");
		return false;
	}
	this->cursors[CURSOR_ADD] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_IBEAM), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	if (this->cursors[CURSOR_ADD] == NULL) {
		DEBUGWRITE("NGMainWnd::init() cannot load add cursor !");
		return false;
	}
	this->cursors[CURSOR_NO] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_NO), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	if (this->cursors[CURSOR_NO] == NULL) {
		DEBUGWRITE("NGMainWnd::init() cannot load no cursor !");
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
		delete this->client;
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

bool NGMainWnd::create(int x, int y, dword width, dword height) {
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

	// create status window
	this->hstatuswnd = CreateWindowEx(0, STATUSCLASSNAME, "StatusWindow", WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, this->hwnd, (HMENU)IDC_STATUSBAR, this->hinstance, NULL);
	if (this->hstatuswnd == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create status window !");
		return false;
	}
	GetWindowRect(this->hstatuswnd, &rc);
	this->statuswidth = rc.right - rc.left;
	this->statusheight = rc.bottom - rc.top;

	// create connect toolbox window
	this->htoolboxwnd[TOOLBOX_CONNECT] = CreateDialog(this->hinstance, MAKEINTRESOURCE(IDD_TOOLBOX_CONNECT), this->hwnd, (DLGPROC)toolboxdlgproc);
	if (this->htoolboxwnd[TOOLBOX_CONNECT] == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create connect toolbox window !");
		return false;
	}
	GetWindowRect(this->htoolboxwnd[TOOLBOX_CONNECT], &rc);
	this->toolboxwidth[TOOLBOX_CONNECT] = rc.right - rc.left; //this->clientwidth;
	this->toolboxheight[TOOLBOX_CONNECT] = rc.bottom - rc.top;
	if (this->toolboxminwidth < this->toolboxwidth[TOOLBOX_CONNECT]) {
		this->toolboxminwidth = this->toolboxwidth[TOOLBOX_CONNECT];
	}
	SetWindowPos(this->htoolboxwnd[TOOLBOX_CONNECT], NULL, 0, 0, this->toolboxwidth[TOOLBOX_CONNECT], this->toolboxheight[TOOLBOX_CONNECT], SWP_NOZORDER);
	SetDlgItemText(this->htoolboxwnd[TOOLBOX_CONNECT], IDC_HOSTNAME, "localhost");
	SetDlgItemText(this->htoolboxwnd[TOOLBOX_CONNECT], IDC_PORT, "8443");
	SetDlgItemText(this->htoolboxwnd[TOOLBOX_CONNECT], IDC_USERNAME, "nitro");
	SetDlgItemText(this->htoolboxwnd[TOOLBOX_CONNECT], IDC_PASSWORD, "abc");

	// create connecting toolbox window
	this->htoolboxwnd[TOOLBOX_CONNECTING] = CreateDialog(this->hinstance, MAKEINTRESOURCE(IDD_TOOLBOX_CONNECTING), this->hwnd, (DLGPROC)toolboxdlgproc);
	if (this->htoolboxwnd[TOOLBOX_CONNECTING] == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create connecting toolbox window !");
		return false;
	}
	GetWindowRect(this->htoolboxwnd[TOOLBOX_CONNECTING], &rc);
	this->toolboxwidth[TOOLBOX_CONNECTING] = rc.right - rc.left; //this->clientwidth;
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
	this->toolboxwidth[TOOLBOX_ADMIN] = rc.right - rc.left; //this->clientwidth;
	this->toolboxheight[TOOLBOX_ADMIN] = rc.bottom - rc.top;
	if (this->toolboxminwidth < this->toolboxwidth[TOOLBOX_ADMIN]) {
		this->toolboxminwidth = this->toolboxwidth[TOOLBOX_ADMIN];
	}
	SetWindowPos(this->htoolboxwnd[TOOLBOX_ADMIN], NULL, 0, 0, this->toolboxwidth[TOOLBOX_ADMIN], this->toolboxheight[TOOLBOX_ADMIN], SWP_NOZORDER);
	SendDlgItemMessage(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_SONG_PROGRESS, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, 1000));
	SendDlgItemMessage(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_SONG_PROGRESS, TBM_SETTICFREQ, 50, 0);
	SendDlgItemMessage(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_VOLUME, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, 100));
	SendDlgItemMessage(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_VOLUME, TBM_SETTICFREQ, 5, 0);
	SendDlgItemMessage(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_PAN, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, 200));
	SendDlgItemMessage(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_PAN, TBM_SETTICFREQ, 5, 0);

	// create user toolbox window
	this->htoolboxwnd[TOOLBOX_USER] = CreateDialog(this->hinstance, MAKEINTRESOURCE(IDD_TOOLBOX_USER), this->hwnd, (DLGPROC)toolboxdlgproc);
	if (this->htoolboxwnd[TOOLBOX_USER] == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create user toolbox window !");
		return false;
	}
	GetWindowRect(this->htoolboxwnd[TOOLBOX_USER], &rc);
	this->toolboxwidth[TOOLBOX_USER] = rc.right - rc.left; //this->clientwidth;
	this->toolboxheight[TOOLBOX_USER] = rc.bottom - rc.top;
	if (this->toolboxminwidth < this->toolboxwidth[TOOLBOX_USER]) {
		this->toolboxminwidth = this->toolboxwidth[TOOLBOX_USER];
	}
	SetWindowPos(this->htoolboxwnd[TOOLBOX_USER], NULL, 0, 0, this->toolboxwidth[TOOLBOX_USER], this->toolboxheight[TOOLBOX_USER], SWP_NOZORDER);

	// select initial toolbox (connect)
	this->toolbox_select(TOOLBOX_CONNECT);

	// create browse window
	this->browsewidth = this->clientwidth / NGWINAMP_HORIZONTAL_RATIO;
	this->browseheight = this->clientheight - this->statusheight - this->toolboxheight[this->curtoolbox];
	this->hbrowsewnd = CreateWindowEx(WS_EX_STATICEDGE, WC_TREEVIEW, "BrowserWindow",
		WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS, 0, this->toolboxheight[this->curtoolbox], this->browsewidth, this->browseheight, this->hwnd, NULL, this->hinstance, NULL);
	if (this->hbrowsewnd == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create browse window !");
		return false;
	}

	// create filelist window
	this->filewidth = this->clientwidth - this->browsewidth - NGWINAMP_BAR_WIDTH;
	this->fileheight = (this->clientheight - this->statusheight - this->toolboxheight[this->curtoolbox]) / NGWINAMP_VERTICAL_RATIO;
	this->hfilewnd = CreateWindowEx(WS_EX_STATICEDGE, WC_LISTVIEW, "FileListWindow",
		WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox], this->filewidth, this->fileheight, this->hwnd, NULL, this->hinstance, NULL);
	if (this->hfilewnd == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create filelist window !");
		return false;
	}
	SendMessage(this->hfilewnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// create playlist window
	this->plwidth = this->clientwidth - this->browsewidth - NGWINAMP_BAR_WIDTH;
	this->plheight = this->clientheight - this->statusheight - this->toolboxheight[this->curtoolbox] - this->fileheight - NGWINAMP_BAR_WIDTH;
	this->hplwnd = CreateWindowEx(WS_EX_STATICEDGE,
		WC_LISTVIEW, "PlayListWindow", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox] - this->fileheight - NGWINAMP_BAR_WIDTH, this->plwidth, this->plheight, this->hwnd, NULL, this->hinstance, NULL);
	if (this->hplwnd == NULL) {
		DEBUGWRITE("NGMainWnd::create() cannot create playlist window !");
		return false;
	}
	SendMessage(this->hplwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// setup initial toolbox
	GetClientRect(this->htoolboxwnd[this->curtoolbox], &rc);
	SetWindowPos(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_TOOLBOX_TITLE), NULL, rc.left + NGWINAMP_TOOLBOX_TITLE_MARGIN + 1, rc.top + NGWINAMP_TOOLBOX_TITLE_MARGIN, rc.right - rc.left - 2 * NGWINAMP_TOOLBOX_TITLE_MARGIN, rc.bottom - rc.top - 2 * NGWINAMP_TOOLBOX_TITLE_MARGIN, SWP_NOZORDER);

	// setup browse window
	TreeView_SetImageList(this->hbrowsewnd, this->himages, TVSIL_NORMAL);

	// setup filelist window
	LVCOLUMN lvc;

	memset(&lvc, 0, sizeof(LVCOLUMN));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.iSubItem = 0;
	lvc.cx = 250;
	lvc.pszText = NGWINAMP_FILE_HEADER_TITLE;
	ListView_InsertColumn(this->hfilewnd, 0, &lvc);
	lvc.iSubItem = 1;
	lvc.cx = 100;
	lvc.pszText = NGWINAMP_FILE_HEADER_SIZE;
	ListView_InsertColumn(this->hfilewnd, 1, &lvc);
	lvc.iSubItem = 2;
	lvc.cx = 150;
	lvc.pszText = NGWINAMP_FILE_HEADER_DATE;
	ListView_InsertColumn(this->hfilewnd, 2, &lvc);
	ListView_SetImageList(this->hfilewnd, this->himages, LVSIL_SMALL);

	// setup playlist window
	memset(&lvc, 0, sizeof(LVCOLUMN));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.iSubItem = 0;
	lvc.cx = 50;
	lvc.pszText = NGWINAMP_PL_HEADER_INDEX;
	ListView_InsertColumn(this->hplwnd, 0, &lvc);
	lvc.iSubItem = 1;
	lvc.cx = 250;
	lvc.pszText = NGWINAMP_PL_HEADER_TITLE;
	ListView_InsertColumn(this->hplwnd, 1, &lvc);
	lvc.iSubItem = 2;
	lvc.cx = 250;
	lvc.pszText = NGWINAMP_PL_HEADER_PATH;
	ListView_InsertColumn(this->hplwnd, 2, &lvc);
	ListView_SetImageList(this->hplwnd, this->himages, LVSIL_SMALL);
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
	if (this->hplwnd != NULL) {
		DestroyWindow(this->hplwnd);
		this->hplwnd = NULL;
	}
	if (this->hfilewnd != NULL) {
		DestroyWindow(this->hfilewnd);
		this->hfilewnd = NULL;
	}
	if (this->hbrowsewnd != NULL) {
		DestroyWindow(this->hbrowsewnd);
		this->hbrowsewnd = NULL;
	}
	if (this->hstatuswnd != NULL) {
		DestroyWindow(this->hstatuswnd);
		this->hstatuswnd = NULL;
	}
	if (this->hwnd != NULL) {
		DestroyWindow(this->hwnd);
		this->hwnd = NULL;
	}
	return true;
}
bool NGMainWnd::main(void) {
	MSG msg;

	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (this->htoolboxwnd[this->curtoolbox] != NULL && IsDialogMessage(this->htoolboxwnd[this->curtoolbox], &msg)) {
				continue;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				break;
			}
		} else if (this->client->isrunning()) {
			if (!this->client->process()) {
				Sleep(5);
			}
		} else {
			Sleep(5);
		}
	}
	return true;
}


bool NGMainWnd::isDragNDrop() {
	if (this->dragndrop != NULL) {
		return true;
	}
	return false;
}

void NGMainWnd::refreshplaylistpos() {
	if (this->playlist_pos >= 0 && this->playlist_pos < (long)this->playlist.size()) {
		string name = this->playlist[this->playlist_pos].name;
		char *buffer = new char[name.length() + 16];

		sprintf(buffer, "%d. %s", this->playlist_pos + 1, name.c_str());
		if (textbox_getstring(this->htoolboxwnd[this->curtoolbox], IDC_SONG_TITLE).compare(buffer) != 0) {
			SetDlgItemText(this->htoolboxwnd[this->curtoolbox], IDC_SONG_TITLE, buffer);
		}
	} else {
		if (textbox_getstring(this->htoolboxwnd[this->curtoolbox], IDC_SONG_TITLE).length() > 0) {
			SetDlgItemText(this->htoolboxwnd[this->curtoolbox], IDC_SONG_TITLE, "");
		}
	}
}

void NGMainWnd::listview_refreshplaylist() {
	dword size = ListView_GetItemCount(this->hplwnd);

	for (dword i = 0; i < this->playlist.size(); i++) {
		char buffer[16];

		sprintf(buffer, "%d.", i + 1);
		if (i >= size) {
			LVITEM lvi;

			memset(&lvi, 0, sizeof(LVITEM));
			lvi.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.pszText = buffer;
			lvi.iImage = ICON_MAIN;
			lvi.lParam = i;
			ListView_InsertItem(this->hplwnd, &lvi);
		} else {
			ListView_SetItemText(this->hplwnd, i, 0, buffer);
		}
		ListView_SetItemText(this->hplwnd, i, 1, (LPSTR)this->playlist[i].name.c_str());
		ListView_SetItemText(this->hplwnd, i, 2, (LPSTR)this->playlist[i].path.c_str());
	}
	for (dword i = this->playlist.size(); i < size; i++) {
		ListView_DeleteItem(this->hplwnd, this->playlist.size());
	}
	this->refreshplaylistpos();
}

void NGMainWnd::listview_setfiles(const vector<FileItem> &files) {
	dword size = ListView_GetItemCount(this->hfilewnd);

	for (dword i = 0; i < files.size(); i++) {
		char csize[64];
		char cdate[64];

		if (files[i].size < 1024) {
			sprintf(csize, "%u [b]", files[i].size);
		} else if (files[i].size < 1024 * 1024) {
			sprintf(csize, "%.01f [kb]", (double)files[i].size / 1024.0);
		} else {
			sprintf(csize, "%.01f [mb]", (double)files[i].size / (1024.0 * 1024.0));
		}
		sprintf(cdate, "?");
		if (i >= size) {
			LVITEM lvi;

			memset(&lvi, 0, sizeof(LVITEM));
			lvi.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.pszText = (LPSTR)files[i].name.c_str();
			lvi.iImage = ICON_MAIN;
			lvi.lParam = i;
			ListView_InsertItem(this->hfilewnd, &lvi);
		} else {
			ListView_SetItemText(this->hfilewnd, i, 0, (LPSTR)files[i].name.c_str());
		}
		ListView_SetItemText(this->hfilewnd, i, 1, csize);
		ListView_SetItemText(this->hfilewnd, i, 2, cdate);
	}
	for (dword i = files.size(); i < size; i++) {
		ListView_DeleteItem(this->hfilewnd, files.size());
	}
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


string NGMainWnd::textbox_getstring(HWND hwnd, int id) {
	string	ret;
	char	*buffer;
	int		length = GetWindowTextLength(GetDlgItem(hwnd, id));

	buffer = new char[length + 1];
	GetDlgItemText(hwnd, id, buffer, length + 1);
	ret = string(buffer);
	delete [] buffer;
	return ret;
}

bool NGMainWnd::checkbox_selected(HWND hwnd, int id) {
	if (IsDlgButtonChecked(hwnd, id) == BST_CHECKED) {
		return true;
	}
	return false;
}

bool NGMainWnd::menuitem_checked(HMENU hmenu, int id) {
	if (GetMenuState(hmenu, id, MF_BYCOMMAND) == MF_CHECKED) {
		CheckMenuItem(hmenu, id, MF_BYCOMMAND | MF_UNCHECKED);
		return false;
	}
	CheckMenuItem(hmenu, id, MF_BYCOMMAND | MF_CHECKED);
	return true;
}

vector<dword> NGMainWnd::listview_getselection(HWND hwnd) {
	vector<dword> indexes;
	dword size = ListView_GetItemCount(hwnd);

	for (dword i = 0; i < size; i++) {
		if (ListView_GetItemState(hwnd, i, LVIS_SELECTED) == LVIS_SELECTED) {
			indexes.push_back(i);
		}
	}
	return indexes;
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

		case WM_SIZING:
			{
				RECT  *rc = (RECT*)lparam;
				dword width = rc->right - rc->left;
				dword height = rc->bottom - rc->top;

				if (pmainwnd->onwnd_sizing(width, height)) {
					switch (wparam) {
					case WMSZ_LEFT:
					case WMSZ_TOPLEFT:
					case WMSZ_BOTTOMLEFT:
						rc->left = rc->right - width;
						break;

					case WMSZ_RIGHT:
					case WMSZ_TOPRIGHT:
					case WMSZ_BOTTOMRIGHT:
						rc->right = rc->left + width;
						break;
					}
					switch (wparam) {
					case WMSZ_TOP:
					case WMSZ_TOPLEFT:
					case WMSZ_TOPRIGHT:
						rc->top = rc->bottom - height;
						break;

					case WMSZ_BOTTOM:
					case WMSZ_BOTTOMLEFT:
					case WMSZ_BOTTOMRIGHT:
						rc->bottom = rc->top + height;
						break;
					}
				} else {
					DEBUGWRITE("mainwndproc() error processing WM_SIZING event !");
					PostQuitMessage(0);
				}
			}
			break;

		case WM_SIZE:
			if (wparam != SIZE_MINIMIZED) {
				if (!pmainwnd->onwnd_resized()) {
					DEBUGWRITE("mainwndproc() error processing WM_SIZE event !");
					PostQuitMessage(0);
				}
			}
			return 0;

		case WM_LBUTTONDOWN:
			if (!pmainwnd->isDragNDrop()) {
				if (!pmainwnd->onwnd_mousemove(LOWORD(lparam), HIWORD(lparam), true)) {
					DEBUGWRITE("mainwndproc() error processing WM_LBUTTONDOWN event !");
					PostQuitMessage(0);
				}
				return 0;
			}
			break;

		case WM_LBUTTONUP:
			if (pmainwnd->isDragNDrop()) {
				if (!pmainwnd->onwnd_mousemove(LOWORD(lparam), HIWORD(lparam), false)) {
					DEBUGWRITE("mainwndproc() error processing WM_LBUTTONUP event !");
					PostQuitMessage(0);
				}
				return 0;
			}
			break;

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

		case WM_NOTIFY:
			if (!pmainwnd->onwnd_notify((dword)wparam, (LPNMHDR)lparam)) {
				DEBUGWRITE("mainwndproc() error processing WM_NOTIFY event !");
				PostQuitMessage(0);
			}
			break;

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

	case WM_HSCROLL:
		if (!pmainwnd->onwnd_hscroll(LOWORD(wparam), HIWORD(wparam), (HWND)lparam)) {
			DEBUGWRITE("mainwndproc() error processing WM_HSCROLL event !");
			PostQuitMessage(0);
		}
		break;

	case WM_NOTIFY:
		if (!pmainwnd->onwnd_notify((dword)wparam, (LPNMHDR)lparam)) {
			DEBUGWRITE("toolboxdlgproc() error processing WM_NOTIFY event !");
			PostQuitMessage(0);
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
