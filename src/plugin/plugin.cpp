// plugin.cpp
#include "plugin.h"
#include "../config.h"
#include "../fs.h"
#include "../netdata.h"
#include "ngwinampserver.h"


NGWINAMP	*PLUGIN::pwinamp = NULL;
HINSTANCE	PLUGIN::hInstance = NULL;


BOOL WINAPI DllMain(HINSTANCE hinstance, ULONG reason, LPVOID reserved) {
	WSADATA wd;

	switch (reason) {
	case DLL_PROCESS_ATTACH:
		DEBUGCLEAR;
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


int  init();
void quit();
void config();

char			name[] = NGWINAMP_NAME;
WINAMP_GENDATA	winamp = {GPPHDR_VER, name, init, config, quit, NULL, NULL};


int init() {
	PLUGIN::hInstance = winamp.hDllInstance;
	if (PLUGIN::pwinamp == NULL) {
		PLUGIN::pwinamp = new NGWINAMPSERVER(winamp.hwndParent);
	}
	if (PLUGIN::pwinamp != NULL) {
		((NGWINAMPSERVER*)PLUGIN::pwinamp)->start();
	}
	return 0;
}

void quit() {
	if (PLUGIN::pwinamp != NULL) {
		delete ((NGWINAMPSERVER*)PLUGIN::pwinamp);
		PLUGIN::pwinamp = NULL;
	}
	PLUGIN::hInstance = NULL;
}

void config() {
	DEBUGWRITE("ngwinamp config()...");
//	openconfig(&PLUGIN::plugin, PLUGIN::plugin.pwinamp->getwinampwnd());
}


extern "C" {
	__declspec(dllexport) HWINAMP_GENDATA winampGetGeneralPurposePlugin() {
		return &winamp;
	}
}
