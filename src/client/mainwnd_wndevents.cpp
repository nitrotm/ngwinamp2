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
bool NGMainWnd::onwnd_sizing(dword &width, dword &height) {
	if (width < this->toolboxminwidth) {
		width = this->toolboxminwidth;
	}
	if (height < NGWINAMP_MIN_HEIGHT) {
		height = NGWINAMP_MIN_HEIGHT;
	}
	return true;
}
bool NGMainWnd::onwnd_resized(void) {
	RECT rc;

	GetClientRect(this->hwnd, &rc);
	this->clientwidth = rc.right - rc.left;
	this->clientheight = rc.bottom - rc.top;

	this->statuswidth = this->clientwidth;
	SetWindowPos(this->hstatuswnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);

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
	this->browseheight = this->clientheight - this->statusheight - this->toolboxheight[this->curtoolbox];
	SetWindowPos(this->hbrowsewnd, NULL, 0, this->toolboxheight[this->curtoolbox], this->browsewidth, this->browseheight, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);

	this->filewidth = this->clientwidth - this->browsewidth - NGWINAMP_BAR_WIDTH;
	if (this->fileheight < (NGWINAMP_BAR_WIDTH + NGWINAMP_MIN_PANEL_HEIGHT)) {
		this->fileheight = NGWINAMP_BAR_WIDTH + NGWINAMP_MIN_PANEL_HEIGHT;
	}
	if (this->fileheight > (this->clientheight - this->statusheight - this->toolboxheight[this->curtoolbox] - NGWINAMP_BAR_WIDTH - NGWINAMP_MIN_PANEL_HEIGHT)) {
		this->fileheight = this->clientheight - this->statusheight - this->toolboxheight[this->curtoolbox] - NGWINAMP_BAR_WIDTH - NGWINAMP_MIN_PANEL_HEIGHT;
	}
	SetWindowPos(this->hfilewnd, NULL, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox], this->filewidth, this->fileheight, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);

	this->plwidth = this->clientwidth - this->browsewidth - NGWINAMP_BAR_WIDTH;
	this->plheight = this->clientheight - this->statusheight - this->toolboxheight[this->curtoolbox] - this->fileheight - NGWINAMP_BAR_WIDTH;
	SetWindowPos(this->hplwnd, NULL, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox] + this->fileheight + NGWINAMP_BAR_WIDTH, this->plwidth, this->plheight, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);
	return true;
}
bool NGMainWnd::onwnd_mousemove(dword mx, dword my, bool mousedown) {
	if (this->dragndrop != NULL) {
		this->lastmx = mx;
		this->lastmy = my;
		if (this->dragndrop->isfilelist()) {
			FileDragNDrop *filedragndrop = (FileDragNDrop*)this->dragndrop;

			ListView_SetItemState(this->hplwnd, -1, 0, LVIS_SELECTED | LVIS_DROPHILITED);
			if (mousedown) {
				if (mx >= (this->browsewidth + NGWINAMP_BAR_WIDTH) && mx < this->clientwidth &&
					my >= (this->toolboxheight[this->curtoolbox] + this->fileheight + NGWINAMP_BAR_WIDTH) && my < (this->clientheight - this->statusheight)) {
					LVHITTESTINFO lvhti;

					memset(&lvhti, 0, sizeof(LVHITTESTINFO));
					lvhti.pt.x = mx - (this->browsewidth + NGWINAMP_BAR_WIDTH);
					lvhti.pt.y = my - (this->toolboxheight[this->curtoolbox] + this->fileheight + NGWINAMP_BAR_WIDTH);
					filedragndrop->index = ListView_HitTest(this->hplwnd, &lvhti);
					if (filedragndrop->index >= 0) {
						ListView_SetItemState(this->hplwnd, filedragndrop->index, LVIS_DROPHILITED, LVIS_DROPHILITED);
					}
					SetCursor(this->cursors[CURSOR_ADD]);
				} else {
					SetCursor(this->cursors[CURSOR_NO]);
				}
			} else {
				if (mx >= (this->browsewidth + NGWINAMP_BAR_WIDTH) && mx < this->clientwidth &&
					my >= (this->toolboxheight[this->curtoolbox] + this->fileheight + NGWINAMP_BAR_WIDTH) && my < (this->clientheight - this->statusheight)) {
					// append the files to the playlist at given index
					if (filedragndrop->index < 0) {
						filedragndrop->index = this->playlist.size();
					}
					if (this->client->isrunning()) {
						this->client->request_pl_addfiles(filedragndrop->index, filedragndrop->filelist);
						this->client->request_pl_getfiles();
						this->client->request_pl_getnames();
					}
				}
				delete this->dragndrop;
				this->dragndrop = NULL;
				ReleaseCapture();
				SetCursor(this->cursors[CURSOR_NORMAL]);
			}
		}
	} else if (this->hresize) {
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

					this->filewidth = this->clientwidth - this->browsewidth - NGWINAMP_BAR_WIDTH;
					if (this->fileheight < (NGWINAMP_BAR_WIDTH + NGWINAMP_MIN_PANEL_HEIGHT)) {
						this->fileheight = NGWINAMP_BAR_WIDTH + NGWINAMP_MIN_PANEL_HEIGHT;
					}
					if (this->fileheight > (this->clientwidth - this->statusheight - this->toolboxheight[this->curtoolbox] - NGWINAMP_BAR_WIDTH - NGWINAMP_MIN_PANEL_HEIGHT)) {
						this->fileheight = this->clientwidth - this->statusheight - this->toolboxheight[this->curtoolbox] - NGWINAMP_BAR_WIDTH - NGWINAMP_MIN_PANEL_HEIGHT;
					}
					SetWindowPos(this->hfilewnd, NULL, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox], this->filewidth, this->fileheight, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);

					this->plwidth = this->clientwidth - this->browsewidth - NGWINAMP_BAR_WIDTH;
					SetWindowPos(this->hplwnd, NULL, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox] + this->fileheight + NGWINAMP_BAR_WIDTH, this->plwidth, this->plheight, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);
				}
			}
		} else {
			this->hresize = false;
			ReleaseCapture();
		}
		SetCursor(this->cursors[CURSOR_HRESIZE]);
	} else if (vresize) {
		if (mousedown) {
			if (my >= 0 && my < this->clientheight) {
				int delta = my - this->lastmy;

				this->lastmx = mx;
				this->lastmy = my;

				if (delta != 0) {
					this->fileheight += delta;
					if (this->fileheight < (NGWINAMP_BAR_WIDTH + NGWINAMP_MIN_PANEL_HEIGHT)) {
						this->fileheight = NGWINAMP_BAR_WIDTH + NGWINAMP_MIN_PANEL_HEIGHT;
					}
					if (this->fileheight > (this->clientheight - this->statusheight - this->toolboxheight[this->curtoolbox] - NGWINAMP_BAR_WIDTH - NGWINAMP_MIN_PANEL_HEIGHT)) {
						this->fileheight = this->clientheight - this->statusheight - this->toolboxheight[this->curtoolbox] - NGWINAMP_BAR_WIDTH - NGWINAMP_MIN_PANEL_HEIGHT;
					}
					SetWindowPos(this->hfilewnd, NULL, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox], this->filewidth, this->fileheight, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);

					this->plheight = this->clientheight - this->statusheight - this->toolboxheight[this->curtoolbox] - this->fileheight - NGWINAMP_BAR_WIDTH;
					SetWindowPos(this->hplwnd, NULL, this->browsewidth + NGWINAMP_BAR_WIDTH, this->toolboxheight[this->curtoolbox] + this->fileheight + NGWINAMP_BAR_WIDTH, this->plwidth, this->plheight, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);
				}
			}
		} else {
			this->vresize = false;
			ReleaseCapture();
		}
		SetCursor(this->cursors[CURSOR_VRESIZE]);
	} else if (mx >= this->browsewidth && mx < (this->browsewidth + NGWINAMP_BAR_WIDTH) && my >= 0 && my < this->clientheight) {
		if (mousedown) {
			this->hresize = true;
			this->lastmx = mx;
			this->lastmy = my;
			SetCapture(this->hwnd);
		}
		SetCursor(this->cursors[CURSOR_HRESIZE]);
	} else if (mx >= this->browsewidth && mx < this->clientwidth && my >= (this->toolboxheight[this->curtoolbox] + this->fileheight) && my < (this->toolboxheight[this->curtoolbox] + this->fileheight + NGWINAMP_BAR_WIDTH)) {
		if (mousedown) {
			this->vresize = true;
			this->lastmx = mx;
			this->lastmy = my;
			SetCapture(this->hwnd);
		}
		SetCursor(this->cursors[CURSOR_VRESIZE]);
	} else {
		SetCursor(this->cursors[CURSOR_NORMAL]);
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

		case TOOLBOX_ADMIN:
			if (this->client->isrunning()) {
				switch (id) {
				case IDM_BACK:
				case IDC_BACK:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_BACK)) {
						this->client->request_sn_prev();
					}
					break;
				case IDM_PLAY:
				case IDC_PLAY:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_PLAY)) {
						this->client->request_sn_play();
					}
					break;
				case IDM_PAUSE:
				case IDC_PAUSE:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_PAUSE)) {
						this->client->request_sn_pause();
					}
					break;
				case IDM_STOP:
				case IDC_STOP:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_STOP)) {
						this->client->request_sn_stop();
					}
					break;
				case IDM_NEXT:
				case IDC_NEXT:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_NEXT)) {
						this->client->request_sn_next();
					}
					break;
				case IDM_REPEAT:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
						bool checked = menuitem_checked(GetSubMenu(this->hmenu, 2), IDM_REPEAT);

						if (checked) {
							CheckDlgButton(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_REPEAT, BST_CHECKED);
						} else {
							CheckDlgButton(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_REPEAT, BST_UNCHECKED);
						}
						this->client->request_pl_setrepeat(checked);
					}
					break;
				case IDC_REPEAT:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
						bool checked = checkbox_selected(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_REPEAT);

						if (checked) {
							CheckMenuItem(GetSubMenu(this->hmenu, 2), IDM_REPEAT, MF_BYCOMMAND | MF_CHECKED);
						} else {
							CheckMenuItem(GetSubMenu(this->hmenu, 2), IDM_REPEAT, MF_BYCOMMAND | MF_UNCHECKED);
						}
						this->client->request_pl_setrepeat(checked);
					}
					break;
				case IDM_SHUFFLE:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
						bool checked = menuitem_checked(GetSubMenu(this->hmenu, 2), IDM_SHUFFLE);

						if (checked) {
							CheckDlgButton(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_SHUFFLE, BST_CHECKED);
						} else {
							CheckDlgButton(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_SHUFFLE, BST_UNCHECKED);
						}
						this->client->request_pl_setshuffle(checked);
					}
					break;
				case IDC_SHUFFLE:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
						bool checked = checkbox_selected(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_SHUFFLE);

						if (checked) {
							CheckMenuItem(GetSubMenu(this->hmenu, 2), IDM_SHUFFLE, MF_BYCOMMAND | MF_CHECKED);
						} else {
							CheckMenuItem(GetSubMenu(this->hmenu, 2), IDM_SHUFFLE, MF_BYCOMMAND | MF_CHECKED);
						}
						this->client->request_pl_setshuffle(checked);
					}
					break;
				case IDM_CLEAR:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
						this->client->request_pl_clear();
						this->client->request_pl_getnames();
						this->client->request_pl_getfiles();
					}
					break;
				case IDM_REMOVE_DEAD:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
						this->client->request_pl_removedead();
						this->client->request_pl_getnames();
						this->client->request_pl_getfiles();
					}
					break;
				case IDM_SORTBYNAME:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
						this->client->request_pl_sortbyname();
						this->client->request_pl_getnames();
						this->client->request_pl_getfiles();
					}
					break;
				case IDM_SORTBYPATH:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
						this->client->request_pl_sortbypath();
						this->client->request_pl_getnames();
						this->client->request_pl_getfiles();
					}
					break;
				case IDM_RANDOMIZE:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
						this->client->request_pl_randomize();
						this->client->request_pl_getnames();
						this->client->request_pl_getfiles();
					}
					break;
				case IDM_REFRESH:
					this->client->request_pl_getnames();
					this->client->request_pl_getfiles();
					break;
				}
			}
		}
	}
	return true;
}
bool NGMainWnd::onwnd_notify(WPARAM id, LPNMHDR hdr) {
	if (hdr->hwndFrom == this->hbrowsewnd) {
		switch (hdr->code) {
		case TVN_SELCHANGED:
			if (this->client->isrunning()) {
				LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)hdr;

				this->browseindex = -1;
				for (dword i = 0; i < this->browsenodes.size(); i++) {
					if (this->browsenodes[i].hnode == pnmtv->itemNew.hItem) {
						this->browseindex = i;
						if (this->browsenodes[i].filesloaded) {
							this->listview_setfiles(this->browsenodes[i].files);
						} else {
							this->client->request_bw_getfiles(this->browsenodes[i].path);
						}
						break;
					}
				}
			}
			break;

		case TVN_BEGINDRAG:
			if (this->client->isrunning() && this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_ADD)) {
				LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)hdr;

				if (this->browseindex >= 0 && this->browseindex < (long)this->browsenodes.size()) {
					vector<string> selectedpaths;

					selectedpaths.push_back(this->browsenodes[this->browseindex].path);
					if (this->dragndrop != NULL) {
						delete this->dragndrop;
						this->dragndrop = NULL;
					}
					this->dragndrop = new FileDragNDrop(
						selectedpaths
					);
					this->lastmx = -1;
					this->lastmy = -1;
					SetCapture(this->hwnd);
				}
			}
			break;
		}
	} else if (hdr->hwndFrom == this->hfilewnd) {
		switch (hdr->code) {
		case LVN_BEGINDRAG:
			if (this->client->isrunning() && this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_ADD)) {
				LPNMLISTVIEW pnmlv = (LPNMLISTVIEW)hdr;

				if (this->browseindex >= 0 && this->browseindex < (long)this->browsenodes.size()) {
					vector<dword> selected = listview_getselection(this->hfilewnd);
					vector<string> selectedpaths;

					for (dword i = 0; i < selected.size(); i++) {
						if (selected[i] < this->browsenodes[this->browseindex].files.size()) {
							selectedpaths.push_back(this->browsenodes[this->browseindex].path + this->browsenodes[this->browseindex].files[selected[i]].name);
						}
					}
					if (this->dragndrop != NULL) {
						delete this->dragndrop;
						this->dragndrop = NULL;
					}
					this->dragndrop = new FileDragNDrop(
						selectedpaths
					);
					this->lastmx = -1;
					this->lastmy = -1;
					SetCapture(this->hwnd);
				}
			}
			break;
		}
	} else if (hdr->hwndFrom == this->hplwnd) {
		switch (hdr->code) {
		case NM_DBLCLK:
			if (this->client->isrunning() && this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
				LPNMITEMACTIVATE pnmia = (LPNMITEMACTIVATE)hdr;

				if (pnmia->iItem >= 0 && pnmia->iItem < (long)this->playlist.size()) {
					this->client->request_pl_setpos(pnmia->iItem);
					this->client->request_sn_play();
				}
			}
			break;

		case LVN_KEYDOWN:
			if (this->client->isrunning()) {
				LPNMLVKEYDOWN plvkd = (LPNMLVKEYDOWN)hdr;

				switch (plvkd->wVKey) {
				case VK_DELETE:
					if (this->client->hasaccess(NGWINAMPUSER_ACCESS_PL_DEL)) {
						vector<dword> selected = listview_getselection(this->hplwnd);

						this->client->request_pl_delfiles(selected);
					}
					break;
				}
			}
			break;
		}
	}
	return true;
}

bool NGMainWnd::onwnd_hscroll(dword code, dword position, HWND hcontrol) {
	switch (this->curtoolbox) {
	case TOOLBOX_ADMIN:
		if (hcontrol == GetDlgItem(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_VOLUME)) {
			if (this->client->isrunning() && this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_VOLUME)) {
				double current = (double)SendMessage(hcontrol, TBM_GETPOS, 0, 0) / 100.0;
				char cbuf[16];

				sprintf(cbuf, "%.00f%%", current * 100.0);
				SetDlgItemText(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_VOLUME_VALUE, cbuf);
				this->client->request_sn_setvolume(current);
			}
		} else if (hcontrol == GetDlgItem(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_PAN)) {
			if (this->client->isrunning() && this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_PAN)) {
				double current = ((double)SendMessage(hcontrol, TBM_GETPOS, 0, 0) - 100.0) / 100.0;
				char cbuf[16];

				sprintf(cbuf, "%.00f%%", current * 100.0);
				SetDlgItemText(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_PAN_VALUE, cbuf);
				this->client->request_sn_setpan(current);
			}
		} else if (hcontrol == GetDlgItem(this->htoolboxwnd[TOOLBOX_ADMIN], IDC_SONG_PROGRESS)) {
			if (this->client->isrunning() && this->client->hasaccess(NGWINAMPUSER_ACCESS_SN_POS)) {
				double current = (double)SendMessage(hcontrol, TBM_GETPOS, 0, 0) / 1000.0;
/*				char cbuf[32];
				int	 currentSecond = current % 60;
				int  currentMinute = ((current - currentSecond) / 60) % 60;
				int  currentHour = (current - currentMinute * 60 - currentSecond) / 3600;
*/
				this->client->request_sn_setpos(current);
			}
		}
		break;
	}
	return true;
}
