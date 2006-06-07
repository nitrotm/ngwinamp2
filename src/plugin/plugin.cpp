// plugin.cpp
#include "../global.h"
#include "../util.h"
#include "../config.h"
#include "../fs.h"
#include "../net.h"
#include "../netdata.h"
#include "../sdk/gen.h"
#include "plugin.h"
#include "ngwinamp.h"
#include "ngwinampserver.h"


int  init(void);
void quit(void);
void config(void);


NGWINAMP					*pwinamp = NULL;
HINSTANCE					hInstance = NULL;
char						name[] = NGWINAMP_NAME;
winampGeneralPurposePlugin	winamp = {GPPHDR_VER, name, init, config, quit, NULL, NULL};


BOOL WINAPI DllMain(HINSTANCE hinstance, ULONG reason, LPVOID reserved) {
	WSADATA wd;

	switch (reason) {
	case DLL_PROCESS_ATTACH:
		DEBUGCLEAR("c:\\gen_ngwinamp.dll.log");
		DEBUGWRITE("DLL attached to winamp");
		WSAStartup(MAKEWORD(2, 0), &wd);
		break;

	case DLL_PROCESS_DETACH:
		DEBUGWRITE("DLL detached from winamp");
		WSACleanup();
		break;
	}
	return TRUE;
}



int init(void) {
	hInstance = winamp.hDllInstance;
	if (pwinamp == NULL) {
		pwinamp = new NGWINAMPSERVER(winamp.hwndParent);
	}
	if (pwinamp != NULL) {
		((NGWINAMPSERVER*)pwinamp)->start();
	}
	return 0;
}

void quit(void) {
	if (pwinamp != NULL) {
		delete ((NGWINAMPSERVER*)pwinamp);
		pwinamp = NULL;
	}
	hInstance = NULL;
}

void config(void) {
	DEBUGWRITE("ngwinamp config()...");
//	openconfig(&PLUGIN::plugin, PLUGIN::plugin.pwinamp->getwinampwnd());
}


extern "C" {
	__declspec(dllexport) winampGeneralPurposePlugin* winampGetGeneralPurposePlugin() {
		return &winamp;
	}
}
