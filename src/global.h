// global.h
#ifndef _GLOBAL_H_INCLUDE_
#define _GLOBAL_H_INCLUDE_


// constantes de configuration
#define NGWINAMP_NAME					"NGWinamp - TCP/IP RC v2.0.0[beta]"
#define NGWINAMP_VERSION_MAJOR			2
#define NGWINAMP_VERSION_MINOR			0


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

using namespace std;


// type definitions
typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;


// local includes
#include "util.h"


#endif //_GLOBAL_H_INCLUDE_
