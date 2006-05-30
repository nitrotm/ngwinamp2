#include "client.h"
#include "mainwnd.h"


NGMainWnd *pmainwnd;



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR Parameters, int ShowValue) {
	INITCOMMONCONTROLSEX icc;

	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_BAR_CLASSES | ICC_INTERNET_CLASSES | ICC_LISTVIEW_CLASSES |
		ICC_PROGRESS_CLASS | ICC_TREEVIEW_CLASSES | ICC_UPDOWN_CLASS;
	if (!InitCommonControlsEx(&icc)) {
		return 1;
	}

	pmainwnd = new NGMainWnd(hInstance);
	if (pmainwnd->init()) {
		if (pmainwnd->create(CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT)) {
			pmainwnd->main();
		}
		pmainwnd->destroy();
	}
	pmainwnd->free();
	delete pmainwnd;
	return 0;
}
