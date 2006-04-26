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
};


#endif //_PLUGIN_H_INCLUDE_
