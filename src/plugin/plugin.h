// plugin.h
#ifndef _PLUGIN_H_INCLUDE_
#define _PLUGIN_H_INCLUDE_


// local includes
#include "../global.h"
#include "api_v2.h"
#include "ngwinamp.h"
#include "ngwinampserver.h"
#include "ngwinampcon.h"
#include "ngwinampuser.h"


#define NGWINAMP_NAME					"NGWinamp - TCP/IP RC v2.0.0[beta]"


// structure principale des plugins génériques
class PLUGIN {
public:
	static NGWINAMPSERVER	*pwinamp;
	static HINSTANCE		hInstance;


	static void cleardebug() {
		CloseHandle(CreateFile("C:\\ngwinamp.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL));
	}

	static void debug(const char *text) {
		HANDLE	hFile = CreateFile("C:\\ngwinamp.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
		DWORD	bw;

		SetFilePointer(hFile, 0, NULL, FILE_END);
		WriteFile(hFile, text, strlen(text), &bw, NULL);
		WriteFile(hFile, "\r\n", 2, &bw, NULL);
		CloseHandle(hFile);
	}
};


#endif //_PLUGIN_H_INCLUDE_
