// global.h
#ifndef _GLOBAL_H_INCLUDE_
#define _GLOBAL_H_INCLUDE_


// constantes de configuration
#define NGWINAMP_MAXANONYMOUSPERMACHINE	2

#define NGWINAMP_NETBUFFERSIZE			4096
#define NGWINAMP_NETBLOCKTIME			1
#define NGWINAMP_NETINITIALTIMEOUT		1000


// constantes génériques
#define NGWINAMP_NONE				0x0
#define NGWINAMP_ALL				0xFFFFFFFF


// windows & std includes
#include <windows.h>
#include <winsock.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include "../res/resource.h"

using namespace std;


// type definitions
typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;


// local includes
#include "util.h"


#endif //_GLOBAL_H_INCLUDE_
