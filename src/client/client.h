// client.h
#ifndef _CLIENT_H_INCLUDE_
#define _CLIENT_H_INCLUDE_


// local includes
#include "../global.h"


// constantes de configuration
#define NGWINAMP_BROWSER_RATIO			3
#define NGWINAMP_BAR_WIDTH				5
#define NGWINAMP_MIN_PANEL_WIDTH		50
#define NGWINAMP_TOOLBOX_TITLE_MARGIN	5

#define NGWINAMP_NETBUFFERSIZE			4096
#define NGWINAMP_NETBLOCKTIME			1
#define NGWINAMP_NETINITIALTIMEOUT		1000

// constantes de localisation
#define NGWINAMP_PL_HEADER_TITLE		"title"
#define NGWINAMP_PL_HEADER_PATH			"path"


string textbox_getstring(HWND hwnd, int id);


#endif //_CLIENT_H_INCLUDE_
