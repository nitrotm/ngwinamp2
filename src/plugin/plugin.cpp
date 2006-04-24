// plugin.cpp
#include "plugin.h"


NGWINAMPSERVER *PLUGIN::pwinamp = NULL;
HINSTANCE		PLUGIN::hInstance = NULL;


int  init();
void quit();
void config();

char			name[] = NGWINAMP_NAME;
WINAMP_GENDATA	winamp = {GPPHDR_VER, name, init, config, quit, NULL, NULL};


int init() {
	PLUGIN::cleardebug();
	PLUGIN::hInstance = winamp.hDllInstance;
	if (PLUGIN::pwinamp == NULL) {
		PLUGIN::pwinamp = new NGWINAMPSERVER(winamp.hwndParent);
	}
	if (PLUGIN::pwinamp != NULL) {
		PLUGIN::pwinamp->start();
	}
	return 0;
}

void quit() {
	if (PLUGIN::pwinamp != NULL) {
		delete PLUGIN::pwinamp;
		PLUGIN::pwinamp = NULL;
	}
	PLUGIN::hInstance = NULL;
}

void config() {
	PLUGIN::debug("ngwinamp config()...");
//	openconfig(&PLUGIN::plugin, PLUGIN::plugin.pwinamp->getwinampwnd());
}


extern "C" {
	__declspec(dllexport) HWINAMP_GENDATA winampGetGeneralPurposePlugin() {
		PLUGIN::debug("ngwinamp winampGetGeneralPurposePlugin()...");
		return &winamp;
	}
}
