// global.h
#ifndef _GLOBAL_H_INCLUDE_
#define _GLOBAL_H_INCLUDE_


/**
  * Global customization
  *
  */
#define NGWINAMP_NAME					"NGWinamp - TCP/IP RC v2.0.0[beta]"
#define NGWINAMP_VERSION_MAJOR			2
#define NGWINAMP_VERSION_MINOR			0

#define NGWINAMP_NETBUFFERSIZE			4096
#define NGWINAMP_NETBLOCKTIME			1
#define NGWINAMP_NETINITIALTIMEOUT		1000


/**
  * Generic constants
  *
  */
#define NGWINAMP_NONE				0x0
#define NGWINAMP_ALL				0xFFFFFFFF


/**
  * Windows & library includes
  *
  */
#include <windows.h>
#include <winsock.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include "zzip/zziplib.h"

using namespace std;


/**
  * Common type definitions
  *
  */
typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;


#endif //_GLOBAL_H_INCLUDE_
