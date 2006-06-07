#include "../global.h"
#include "../util.h"
#include "../net.h"
#include "../netaddr.h"
#include "../netdata.h"
#include "../netauth.h"
#include "client.h"
#include "ngwinampclient.h"
#include "mainwnd.h"


NGMainWnd *pmainwnd;



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR Parameters, int ShowValue) {
	INITCOMMONCONTROLSEX icc;
	WSADATA				 wd;
	int					 ret = 0;

	DEBUGCLEAR("c:\\ngwinampcl.exe.log");
	DEBUGWRITE("WinMain() client startup");

	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_BAR_CLASSES | ICC_INTERNET_CLASSES | ICC_LISTVIEW_CLASSES |
		ICC_PROGRESS_CLASS | ICC_TREEVIEW_CLASSES | ICC_UPDOWN_CLASS |
		ICC_PAGESCROLLER_CLASS;
	if (InitCommonControlsEx(&icc)) {
		pmainwnd = new NGMainWnd(hInstance);
		if (pmainwnd != NULL) {
			DEBUGWRITE("WinMain() main window interface created");

			WSAStartup(MAKEWORD(2, 0), &wd);
			DEBUGWRITE("WinMain() winsock dll loaded");

			if (pmainwnd->init()) {
				DEBUGWRITE("WinMain() main window initialized");
				if (pmainwnd->create(CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT)) {
					DEBUGWRITE("WinMain() main window created, entering main loop...");
					pmainwnd->main();
					DEBUGWRITE("WinMain() main window closed, exit main loop...");
				} else {
					DEBUGWRITE("WinMain() failed to create main window !");
				}
				pmainwnd->destroy();
				DEBUGWRITE("WinMain() main window destroyed");
			} else {
				DEBUGWRITE("WinMain() failed to initialize main window !");
				ret = 3;
			}

			WSACleanup();
			DEBUGWRITE("WinMain() winsock dll freed");

			pmainwnd->free();
			delete pmainwnd;
			DEBUGWRITE("WinMain() main window interface freed");
		} else {
			DEBUGWRITE("WinMain() failed to create main window interface !");
			ret = 2;
		}
	} else {
		DEBUGWRITE("WinMain() failed to initialize common controls !");
		ret = 1;
	}
	DEBUGWRITE("WinMain() client exit");
	return ret;
}
