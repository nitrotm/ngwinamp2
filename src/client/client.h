// client.h
#ifndef _CLIENT_H_INCLUDE_
#define _CLIENT_H_INCLUDE_


// local includes
#include "../global.h"


// constantes de configuration
#define NGWINAMP_HORIZONTAL_RATIO		3
#define NGWINAMP_VERTICAL_RATIO			2
#define NGWINAMP_BAR_WIDTH				5
#define NGWINAMP_MIN_HEIGHT				400
#define NGWINAMP_MIN_PANEL_WIDTH		50
#define NGWINAMP_MIN_PANEL_HEIGHT		50
#define NGWINAMP_TOOLBOX_TITLE_MARGIN	5

#define NGWINAMP_NETBUFFERSIZE			4096
#define NGWINAMP_NETBLOCKTIME			1
#define NGWINAMP_NETINITIALTIMEOUT		1000

// constantes de localisation
#define NGWINAMP_BROWSER_ROOT			"shares"

#define NGWINAMP_PL_HEADER_TITLE		"title"
#define NGWINAMP_PL_HEADER_PATH			"path"

#define NGWINAMP_FILE_HEADER_TITLE		"title"
#define NGWINAMP_FILE_HEADER_SIZE		"size"
#define NGWINAMP_FILE_HEADER_DATE		"date"


string textbox_getstring(HWND hwnd, int id);


#endif //_CLIENT_H_INCLUDE_
