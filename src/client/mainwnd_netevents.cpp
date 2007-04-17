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


bool NGMainWnd::onnet_connected(bool success, const string &title, const string &message) {
	EnableMenuItem(GetSubMenu(this->hmenu, 0), IDM_CONNECT, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(GetSubMenu(this->hmenu, 0), IDM_DISCONNECT, MF_BYCOMMAND | MF_ENABLED);

	if (success) {
		SetDlgItemText(this->htoolboxwnd[TOOLBOX_CONNECTING], IDC_MESSAGE, "request authentication...");
		return this->client->authenticate(textbox_getstring(this->htoolboxwnd[TOOLBOX_CONNECT], IDC_USERNAME), textbox_getstring(this->htoolboxwnd[TOOLBOX_CONNECT], IDC_PASSWORD));
	}
	MessageBox(this->hwnd, message.c_str(), title.c_str(), MB_ICONERROR);
	return false;
}

bool NGMainWnd::onnet_disconnect(void) {
	EnableMenuItem(GetSubMenu(this->hmenu, 0), IDM_CONNECT, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(GetSubMenu(this->hmenu, 0), IDM_DISCONNECT, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(this->hmenu, 1, MF_BYPOSITION | MF_GRAYED);
	EnableMenuItem(this->hmenu, 2, MF_BYPOSITION | MF_GRAYED);
	DrawMenuBar(this->hwnd);

	ListView_DeleteAllItems(this->hplwnd);
	ListView_DeleteAllItems(this->hfilewnd);
	TreeView_DeleteAllItems(this->hbrowsewnd);

	this->playlist.clear();
	this->playlist_pos = -1;
	this->playlist_size = 0;
	this->browsenodes.clear();
	this->browseindex = -1;
	if (this->dragndrop != NULL) {
		delete this->dragndrop;
		this->dragndrop = NULL;
	}
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
bool NGMainWnd::onnet_authsuccess(const NETAUTHEX &auth) {
	EnableMenuItem(this->hmenu, 1, MF_BYPOSITION | MF_ENABLED);
	EnableMenuItem(this->hmenu, 2, MF_BYPOSITION | MF_ENABLED);
	DrawMenuBar(this->hwnd);

	if (this->client->hasaccess(NGWINAMPUSER_ACCESS_READ)) {
		if (this->client->hasaccess(NGWINAMPUSER_ACCESS_WRITE)) {
			this->toolbox_select(TOOLBOX_ADMIN);
			if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_ADD)) {
			} else {
			}
			if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_SET)) {
			} else {
			}
			if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_DEL)) {
			} else {
			}
			if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_SHUFFLE), TRUE);
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_REPEAT), TRUE);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_REPEAT, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_SHUFFLE, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_CLEAR, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_REMOVE_DEAD, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_SORTBYNAME, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_SORTBYPATH, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_RANDOMIZE, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_REFRESH, MF_BYCOMMAND | MF_ENABLED);
			} else {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_SHUFFLE), FALSE);
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_REPEAT), FALSE);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_REPEAT, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_SHUFFLE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_CLEAR, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_REMOVE_DEAD, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_SORTBYNAME, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_SORTBYPATH, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_RANDOMIZE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(GetSubMenu(this->hmenu, 2), IDM_REFRESH, MF_BYCOMMAND | MF_GRAYED);
			}
			if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_BACK)) {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_BACK), TRUE);
				EnableMenuItem(GetSubMenu(this->hmenu, 1), IDM_BACK, MF_BYCOMMAND | MF_ENABLED);
			} else {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_BACK), FALSE);
				EnableMenuItem(GetSubMenu(this->hmenu, 1), IDM_BACK, MF_BYCOMMAND | MF_GRAYED);
			}
			if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_PLAY)) {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_PLAY), TRUE);
				EnableMenuItem(GetSubMenu(this->hmenu, 1), IDM_PLAY, MF_BYCOMMAND | MF_ENABLED);
			} else {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_PLAY), FALSE);
				EnableMenuItem(GetSubMenu(this->hmenu, 1), IDM_PLAY, MF_BYCOMMAND | MF_GRAYED);
			}
			if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_PAUSE)) {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_PAUSE), TRUE);
				EnableMenuItem(GetSubMenu(this->hmenu, 1), IDM_PAUSE, MF_BYCOMMAND | MF_ENABLED);
			} else {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_PAUSE), FALSE);
				EnableMenuItem(GetSubMenu(this->hmenu, 1), IDM_PAUSE, MF_BYCOMMAND | MF_GRAYED);
			}
			if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_STOP)) {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_STOP), TRUE);
				EnableMenuItem(GetSubMenu(this->hmenu, 1), IDM_STOP, MF_BYCOMMAND | MF_ENABLED);
			} else {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_STOP), FALSE);
				EnableMenuItem(GetSubMenu(this->hmenu, 1), IDM_STOP, MF_BYCOMMAND | MF_GRAYED);
			}
			if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_NEXT)) {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_NEXT), TRUE);
				EnableMenuItem(GetSubMenu(this->hmenu, 1), IDM_NEXT, MF_BYCOMMAND | MF_ENABLED);
			} else {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_NEXT), FALSE);
				EnableMenuItem(GetSubMenu(this->hmenu, 1), IDM_NEXT, MF_BYCOMMAND | MF_GRAYED);
			}
			if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_VOLUME)) {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_VOLUME), TRUE);
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_VOLUME_VALUE), TRUE);
			} else {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_VOLUME), FALSE);
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_VOLUME_VALUE), FALSE);
			}
			if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_PAN)) {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_PAN), TRUE);
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_PAN_VALUE), TRUE);
			} else {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_PAN), FALSE);
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_PAN_VALUE), FALSE);
			}
			if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_POS)) {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_SONG_PROGRESS), TRUE);
			} else {
				EnableWindow(GetDlgItem(this->htoolboxwnd[this->curtoolbox], IDC_SONG_PROGRESS), FALSE);
			}
		} else {
			this->toolbox_select(TOOLBOX_USER);
		}
		if (this->client->hasaccess(NGWINAMPUSER_ACCESS_ADMIN)) {
		} else {
		}

		// setup playlist
		if (!this->client->request_pl_getnames()) {
			return false;
		}
		if (!this->client->request_pl_getfiles()) {
			return false;
		}

		// setup initial values
		if (!this->client->request_snapshot(NGWINAMP_SNAPSHOT_REFRESH)) {
			return false;
		}

		// setup share tree
		TVINSERTSTRUCT tvi;

		memset(&tvi, 0, sizeof(TVINSERTSTRUCT));
		tvi.hParent = TVI_ROOT;
		tvi.hInsertAfter = TVI_ROOT;
		tvi.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_TEXT;
		tvi.itemex.cChildren = 1;
		tvi.itemex.pszText = NGWINAMP_BROWSER_ROOT;
		tvi.itemex.iImage = ICON_MAIN;
		this->browsenodes.push_back(PathNode(NULL, TreeView_InsertItem(this->hbrowsewnd, &tvi), "/", 0));
		if (!this->client->request_bw_getdirectories("/")) {
			return false;
		}
		if (!this->client->request_bw_getfiles("/")) {
			return false;
		}
	}
	return true;
}
bool NGMainWnd::onnet_setvolume(double volume) {
	if (this->curtoolbox == TOOLBOX_USER || this->curtoolbox == TOOLBOX_ADMIN) {
		char cbuf[16];

		if (this->curtoolbox == TOOLBOX_ADMIN) {
			SendDlgItemMessage(this->htoolboxwnd[this->curtoolbox], IDC_VOLUME, TBM_SETPOS, TRUE, (int)(volume * 100.0));
		}
		sprintf(cbuf, "%.00f%%", volume * 100.0);
		SetDlgItemText(this->htoolboxwnd[this->curtoolbox], IDC_VOLUME_VALUE, cbuf);
		return true;
	}
	return false;
}
bool NGMainWnd::onnet_setpan(double pan) {
	if (this->curtoolbox == TOOLBOX_USER || this->curtoolbox == TOOLBOX_ADMIN) {
		char cbuf[16];

		if (this->curtoolbox == TOOLBOX_ADMIN) {
			SendDlgItemMessage(this->htoolboxwnd[this->curtoolbox], IDC_PAN, TBM_SETPOS, TRUE, (int)(pan * 100.0) + 100);
		}
		sprintf(cbuf, "%.00f%%", pan * 100.0);
		SetDlgItemText(this->htoolboxwnd[this->curtoolbox], IDC_PAN_VALUE, cbuf);
		return true;
	}
	return false;
}
bool NGMainWnd::onnet_setposition(double progress, dword current, dword length) {
	current /= 1000;
	length /= 1000;
	if (this->curtoolbox == TOOLBOX_USER || this->curtoolbox == TOOLBOX_ADMIN) {
		char cbuf[32];
		int	 currentSecond = current % 60;
		int  currentMinute = ((current - currentSecond) / 60) % 60;
		int  currentHour = (current - currentMinute * 60 - currentSecond) / 3600;
		int	 lengthSecond = length % 60;
		int  lengthMinute = ((length - lengthSecond) / 60) % 60;
		int  lengthHour = (length - lengthMinute * 60 - lengthSecond) / 3600;

		if (this->curtoolbox == TOOLBOX_USER) {
			SendDlgItemMessage(this->htoolboxwnd[this->curtoolbox], IDC_SONG_PROGRESS, PBM_SETRANGE32, 0, length);
			SendDlgItemMessage(this->htoolboxwnd[this->curtoolbox], IDC_SONG_PROGRESS, PBM_SETPOS, current, 0);
		}
		if (this->curtoolbox == TOOLBOX_ADMIN) {
			SendDlgItemMessage(this->htoolboxwnd[this->curtoolbox], IDC_SONG_PROGRESS, TBM_SETPOS, TRUE, (int)(progress * 1000.0));
		}
		sprintf(cbuf, "%02u:%02u:%02u / %02u:%02u:%02u", currentHour, currentMinute, currentSecond, lengthHour, lengthMinute, lengthSecond);
		SetDlgItemText(this->htoolboxwnd[this->curtoolbox], IDC_SONG_POSITION, cbuf);
		return true;
	}
	return false;
}
bool NGMainWnd::onnet_setshuffle(bool shuffle) {
	if (this->curtoolbox == TOOLBOX_USER || this->curtoolbox == TOOLBOX_ADMIN) {
		if (shuffle) {
			CheckDlgButton(this->htoolboxwnd[this->curtoolbox], IDC_SHUFFLE, BST_CHECKED);
			CheckMenuItem(GetSubMenu(this->hmenu, 2), IDM_SHUFFLE, MF_BYCOMMAND | MF_CHECKED);
		} else {
			CheckDlgButton(this->htoolboxwnd[this->curtoolbox], IDC_SHUFFLE, BST_UNCHECKED);
			CheckMenuItem(GetSubMenu(this->hmenu, 2), IDM_SHUFFLE, MF_BYCOMMAND | MF_UNCHECKED);
		}
		return true;
	}
	return false;
}
bool NGMainWnd::onnet_setrepeat(bool repeat) {
	if (this->curtoolbox == TOOLBOX_USER || this->curtoolbox == TOOLBOX_ADMIN) {
		if (repeat) {
			CheckDlgButton(this->htoolboxwnd[this->curtoolbox], IDC_REPEAT, BST_CHECKED);
			CheckMenuItem(GetSubMenu(this->hmenu, 2), IDM_REPEAT, MF_BYCOMMAND | MF_CHECKED);
		} else {
			CheckDlgButton(this->htoolboxwnd[this->curtoolbox], IDC_REPEAT, BST_UNCHECKED);
			CheckMenuItem(GetSubMenu(this->hmenu, 2), IDM_REPEAT, MF_BYCOMMAND | MF_UNCHECKED);
		}
		return true;
	}
	return false;
}

bool NGMainWnd::onnet_directories(const string &path, const vector<string> &directories, const vector<dword> &types, const vector<dword> &childrens) {
	HTREEITEM hparent = NULL;

	// find the parent treenode in the path list
	for (dword i = 0; i < browsenodes.size(); i++) {
		if (browsenodes[i].path.compare(path) == 0) {
			hparent = browsenodes[i].hnode;
			break;
		}
	}
	if (hparent != NULL) {
		// delete previous children
		for (long i = (browsenodes.size() - 1); i >= 0; i--) {
			if (browsenodes[i].hparent == hparent) {
				TreeView_DeleteItem(this->hbrowsewnd, browsenodes[i].hnode);
				browsenodes.erase(browsenodes.begin() + i);
			}
		}

		// append new children
		for (dword i = 0; i < directories.size(); i++) {
			if (childrens[i] > 0) {
				this->client->request_bw_getdirectories(path + directories[i]);
			}

			// update tree items
			TVINSERTSTRUCT tvi;

			memset(&tvi, 0, sizeof(TVINSERTSTRUCT));
			tvi.hParent = hparent;
			tvi.hInsertAfter = TVI_LAST;
			tvi.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_TEXT;
			if (childrens[i] > 0) {
				tvi.itemex.cChildren = 1;
			} else {
				tvi.itemex.cChildren = 0;
			}
			tvi.itemex.pszText = (LPSTR)directories[i].c_str();
			tvi.itemex.iImage = ICON_MAIN;
			this->browsenodes.push_back(PathNode(hparent, TreeView_InsertItem(this->hbrowsewnd, &tvi), path + directories[i] + "/", types[i]));
		}

		// if root, auto-expand
		if (path.compare("/") == 0) {
			TreeView_Expand(this->hbrowsewnd, hparent, TVE_EXPAND);
		}
	}
	return true;
}

bool NGMainWnd::onnet_files(const string &path, const vector<string> &files, const vector<dword> &types, const vector<dword> &sizes) {
	HTREEITEM hparent = NULL;

	// find the parent treenode in the path list
	for (dword i = 0; i < browsenodes.size(); i++) {
		if (browsenodes[i].path.compare(path) == 0) {
			// append new files
			browsenodes[i].files.clear();
			for (dword j = 0; j < files.size(); j++) {
				browsenodes[i].files.push_back(FileItem(files[j], types[j], sizes[j]));
			}
			browsenodes[i].filesloaded = true;

			// refresh files ?
			if (TreeView_GetSelection(this->hbrowsewnd) == browsenodes[i].hnode) {
				listview_setfiles(browsenodes[i].files);
			}
			break;
		}
	}
	return true;
}

bool NGMainWnd::onnet_playlistpos(const dword index, const dword size) {
	this->playlist_pos = index;
	this->playlist_size = size;
	this->refreshplaylistpos();
	return true;
}

bool NGMainWnd::onnet_playlistnames(const vector<string> &names, const vector<dword> &indexes, const dword total) {
	if (this->playlist.size() > total) {
		this->playlist.erase(this->playlist.begin() + total, this->playlist.end());
	}
	for (dword i = 0; i < names.size(); i++) {
		if (indexes[i] < this->playlist.size()) {
			this->playlist[indexes[i]].name = names[i];
		} else if (indexes[i] < total) {
			for (dword j = this->playlist.size(); j < indexes[i]; j++) {
				this->playlist.push_back(PlayListItem("?", "?"));
			}
			this->playlist.push_back(PlayListItem(names[i], "?"));
		}
	}
	this->listview_refreshplaylist();
	return true;
}

bool NGMainWnd::onnet_playlistfiles(const vector<string> &files, const vector<dword> &indexes, const dword total) {
	if (this->playlist.size() > total) {
		this->playlist.erase(this->playlist.begin() + total, this->playlist.end());
	}
	for (dword i = 0; i < files.size(); i++) {
		if (indexes[i] < this->playlist.size()) {
			this->playlist[indexes[i]].path = files[i];
		} else if (indexes[i] < total) {
			for (dword j = this->playlist.size(); j < indexes[i]; j++) {
				this->playlist.push_back(PlayListItem("?", "?"));
			}
			this->playlist.push_back(PlayListItem("?", files[i]));
		}
	}
	this->listview_refreshplaylist();
	return true;
}
