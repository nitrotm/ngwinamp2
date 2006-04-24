// global.h
#ifndef _GLOBAL_H_INCLUDE_
#define _GLOBAL_H_INCLUDE_


// configuration
#define NGWINAMP_MAXANONYMOUSPERMACHINE	2

#define NGWINAMP_NETBUFFERSIZE			4096
#define NGWINAMP_NETBLOCKTIME			1
#define NGWINAMP_NETINITIALTIMEOUT		1000


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

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct hostent SOCKHOSTENT;


// local includes
#include "util.h"
#include "net.h"
#include "fs.h"
#include "config.h"
#include "../res/resource.h"



/*
// prototypes
bool    ngwinamp_init(HMODULE hmodule, bool serverside);
bool    ngwinamp_free(bool saveconfig, bool serverside);
int		config_cl_load(HNGWINAMP_PARAM param);
int     config_sv_load(HNGWINAMP_PARAM param);
int     config_cl_save(HNGWINAMP_PARAM param);
int     config_sv_save(HNGWINAMP_PARAM param);
bool	ngwinamp_addroots(HNGWINAMP_PARAM param, char* proot);
bool	ngwinamp_refreshroots(HNGWINAMP_PARAM param, bool force = false);
bool	ngwinamp_parsedirs(HNGWINAMP_PARAM param, const char *pdirs);
bool	ngwinamp_direxists(HNGWINAMP_PARAM param, const char *pdir);
bool    openconfig(HNGWINAMP_PARAM param, HWND hparentwnd);
bool    openhelp(HNGWINAMP_PARAM param, HWND hparentwnd);

// gui tools
LPARAM	listview_getlparam(HWND hlist, long index);
long	listview_getsel(HWND hlist);
void	listview_getsels(HWND hlist, vector<long> &longs);
long	listview_findindex(HWND hlist, LPARAM lparam);
void	listview_update(HWND hlist, long index, long subindex, char* text);
void	listview_setimage(HWND hlist, long index, long image);
void	combobox_setheight(HWND hcombo, dword height);

// prototypes-serveurs
BOOL CALLBACK configdlgproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK helpdlgproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
int  serverthread(HNGWINAMP_PARAM param);
int  serverconthread(HNETUSER puser);
bool serverproc(HNETUSER puser, HNETDATA pdata);

// prototypes-clients (admin)
BOOL CALLBACK admindlgproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
int  adminthread(HNGWINAMP_PARAM param);
bool admincon(HNETUSER puser);

// prototypes-clients (user)
BOOL CALLBACK userdlgproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
int  userthread(HNGWINAMP_PARAM param);
bool usercon(HNETUSER puser);

// prototypes-clients (browser)
BOOL CALLBACK browserdlgproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
void browserupdate(HNGWINAMP_PARAM param);
void browserupdateroots(HNGWINAMP_PARAM param, HNETDATA pdata);
void browserupdatelist(HNGWINAMP_PARAM param, HNETDATA pdata);
void browsersendfiles(HNGWINAMP_PARAM param);


inline void PostDlgItemMessage(HWND hwnd, dword id, UINT message, WPARAM wparam, LPARAM lparam)
{
	PostMessage(GetDlgItem(hwnd, id), message, wparam, lparam);
}
*/

#endif //_GLOBAL_H_INCLUDE_
