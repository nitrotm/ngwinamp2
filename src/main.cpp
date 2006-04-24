// main.cpp
#include "global.h"


BOOL WINAPI DllMain(HINSTANCE hinstance, ULONG reason, LPVOID reserved) {
	WSADATA wd;

	switch (reason) {
	case DLL_PROCESS_ATTACH:
		WSAStartup(MAKEWORD(2, 0), &wd);
		break;

	case DLL_PROCESS_DETACH:
		WSACleanup();
		break;
	}
	return TRUE;
}



/*
// initialise le plugin
bool ngwinamp_init(HMODULE hmodule, bool serverside)
{
	INITCOMMONCONTROLSEX ccex;


	// initialisation des contrôles communs
	ccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ccex.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&ccex);

	try
	{
		NGSYSTEMPARAM_WIN32 sysparam;

		// active la librairie
#ifdef _DEBUG

		NGERROR::settitle("NGWINAMP V1.x (debug message) - %s");
		NGERROR::setshow(true, NGDEBUG_ERROR | NGDEBUG_WARNING);
		NGERROR::setwrite(true, NGDEBUG_ALL);

#else

		NGERROR::setshow(false, NGDEBUG_NONE);
		NGERROR::setwrite(true, NGDEBUG_ALL); //NGDEBUG_INFOS

#endif

		NGBASE::core_setsynchronize(true);
		NGBASE::core_begin();
		NGBASE::core_registerhandler(NGBASE_ERRORHANDLER, ngwinamp_errhandler);
		NGBASE::core_registerhandler(NGBASE_ALLOCHANDLER, ngwinamp_allochandler);
		NGBASE::core_registerhandler(NGBASE_FREEHANDLER, ngwinamp_freehandler);

		// paramètres systèmes
		sysparam.hmainwnd = NULL;
		sysparam.hmodule = hmodule;
		NGBASE::core_getsystem()->params_set(&sysparam);

		// chargement de la configuration
		PLUGIN::plugin.pdirlist = new NGORGARRAY();
		PLUGIN::plugin.curpath = new NGCNTSTRING(NGBASE::core_getsystem()->app_getdirectory());
		PLUGIN::plugin.pcfg = new NGCFGFILE();
		if (serverside) {
			config_sv_load(&PLUGIN::plugin);
		} else {
			config_cl_load(&PLUGIN::plugin);
		}

		// initialisation globale
		PLUGIN::plugin.s_access = NGBASE::core_getsystem()->sync_create();
		PLUGIN::plugin.hinstance = hmodule;
		PLUGIN::plugin.esharerefresh = NGBASE::core_getsystem()->event_create(false);
		PLUGIN::plugin.erunning = NGBASE::core_getsystem()->event_create(false);
		PLUGIN::plugin.equit = NGBASE::core_getsystem()->event_create(false);
		if(!PLUGIN::plugin.isplugin)
		{
			PLUGIN::plugin.hiconmain = (HICON)LoadImage(PLUGIN::plugin.hinstance, MAKEINTRESOURCE(IDI_SONG), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			PLUGIN::plugin.hicondefault = (HICON)LoadImage(PLUGIN::plugin.hinstance, MAKEINTRESOURCE(IDI_DEFAULT), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			PLUGIN::plugin.hiconcurrent = (HICON)LoadImage(PLUGIN::plugin.hinstance, MAKEINTRESOURCE(IDI_CURRENT), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			PLUGIN::plugin.hicondirectory = (HICON)LoadImage(PLUGIN::plugin.hinstance, MAKEINTRESOURCE(IDI_DIRECTORY), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			PLUGIN::plugin.hiconsong = (HICON)LoadImage(PLUGIN::plugin.hinstance, MAKEINTRESOURCE(IDI_SONG), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

			PLUGIN::plugin.himages = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 4);
			ImageList_AddIcon(PLUGIN::plugin.himages, PLUGIN::plugin.hicondefault);
			ImageList_AddIcon(PLUGIN::plugin.himages, PLUGIN::plugin.hiconcurrent);
			ImageList_AddIcon(PLUGIN::plugin.himages, PLUGIN::plugin.hicondirectory);
			ImageList_AddIcon(PLUGIN::plugin.himages, PLUGIN::plugin.hiconsong);
		}
	}
	catch(...)
	{
		return false;
	}
	return true;
}

// libère le plugin
bool ngwinamp_free(bool saveconfig, bool serverside)
{
	try
	{
		try
		{
			NGWINAMPLOCK lock(&PLUGIN::plugin);

			// sauvegarde
			if(saveconfig) {
				if (serverside) {
					config_sv_save(&PLUGIN::plugin);
				} else {
					config_cl_save(&PLUGIN::plugin);
				}
			}
		}
		catch(...)
		{
		}

		// libération
		NGBASE::core_getsystem()->sync_destroy(PLUGIN::plugin.s_access);
		NGBASE::core_getsystem()->event_destroy(PLUGIN::plugin.esharerefresh);
		NGBASE::core_getsystem()->event_destroy(PLUGIN::plugin.erunning);
		NGBASE::core_getsystem()->event_destroy(PLUGIN::plugin.equit);
		if(!PLUGIN::plugin.isplugin)
		{
			ImageList_Destroy(PLUGIN::plugin.himages);
			DestroyIcon(PLUGIN::plugin.hiconmain);
			DestroyIcon(PLUGIN::plugin.hicondefault);
			DestroyIcon(PLUGIN::plugin.hiconcurrent);
			DestroyIcon(PLUGIN::plugin.hicondirectory);
			DestroyIcon(PLUGIN::plugin.hiconsong);
		}
		delete PLUGIN::plugin.curpath;
		delete PLUGIN::plugin.pcfg;
		delete PLUGIN::plugin.pdirlist;

		// fermeture de la librairie
		NGBASE::core_unregisterhandler(NGBASE_ERRORHANDLER, ngwinamp_errhandler);
		NGBASE::core_unregisterhandler(NGBASE_ALLOCHANDLER, ngwinamp_allochandler);
		NGBASE::core_unregisterhandler(NGBASE_FREEHANDLER, ngwinamp_freehandler);
		NGBASE::core_end();
	}
	catch(...)
	{
		return false;
	}
	return true;
}






bool ngwinamp_direxists(HNGWINAMP_PARAM param, const char *pdir)
{
	NGWINAMPLOCK lock(param);

	for (long j = 0; j < ngstrlen(pdir); j++) {
		if (pdir[j] == '\\') {
			if (j < (ngstrlen(pdir) - 3)) {
				if (pdir[j + 1] == '.' && pdir[j + 2] == '.' && pdir[j + 3] == '\\') {
					return false;
				}
			}
			if (j == (ngstrlen(pdir) - 3)) {
				if (pdir[j + 1] == '.' && pdir[j + 2] == '.') {
					return false;
				}
			}
		}
	}
	for (dword i = 0; i < param->pdirlist->array_getlength(); i++) {
		HNGCNTSTRING ps = (HNGCNTSTRING)param->pdirlist->array_get(i);

		if(ngstrlen(pdir) >= (int)ps->buffer_getlength())
		{
			NGCNTSTRING subpath;

			subpath.buffer_set(0, (char*)pdir, ps->buffer_getlength());
			if(ngstrcmpi(subpath.buffer_get(), ps->buffer_get()) == 0)
				return true;
		}
	}
	return false;
}

bool ngwinamp_parsedirs(HNGWINAMP_PARAM param, const char *pdirs)
{
	NGWINAMPLOCK lock(param);
	int			 lasti = 0;

	param->pdirlist->array_flush();
	for (int i = 0; i <= ngstrlen(pdirs); i++) {
		if (pdirs[i] == ',' || pdirs[i] == 0) {
			NGCNTSTRING dir;

			dir.buffer_set(0, (char*)&pdirs[lasti], i - lasti);
			lasti = i + 1;
			if (!ngwinamp_direxists(param, dir.buffer_get())) {
				if (dir.buffer_getlength() > 0) {
					param->pdirlist->array_ins(param->pdirlist->array_getlength(), &dir);
				}
			}
		}
	}
	return true;
}

bool ngwinamp_addroots(HNGWINAMP_PARAM param, char* proot)
{
	dword count = 0;

	for(dword i = 0; i < (unsigned)ngstrlen(proot); i++)
	{
		if(proot[i] == '\\')
			count++;
	}
	if(proot[0] != '\\' || proot[1] != '\\' || count > 3)
		return true;

	try
	{
		NGCNTSTRING	 s(proot);
		NGWINAMPLOCK lock(param);

		for(i = 0; i < param->psharelist->array_getlength(); i++)
		{
			HNGCNTSTRING ps = (HNGCNTSTRING)param->psharelist->array_get(i);

			if(ngstrcmpi(proot, ps->buffer_get()) == 0)
				return true;
		}
		param->psharelist->array_ins(param->psharelist->array_getlength(), &s);
	}
	catch(...)
	{
		return false;
	}
	return true;
}

int refreshroots_thread(HNGWINAMP_PARAM param)
{
	try
	{
		NGORGARRAY	 list;
		HNGFILE		 pfile = NGFILE::file_alloc();

		NGBASE::core_getsystem()->event_set(param->esharerefresh, true);

		NGASDEVELOP("ngwinamp_refreshroots()", "refreshing network shares : begin")
		pfile->path_getlist("", "", &list, NGFILE_LISTSHARES);
		NGASDEVELOP("ngwinamp_refreshroots()", "refreshing network shares : end")

		for(dword i = 0; i < list.array_getlength(); i++)
		{
			HNGCNTSTRING ps = (HNGCNTSTRING)list.array_get(i);

			if(ps->buffer_getlength() > 0)
				ngwinamp_addroots(param, ps->buffer_get());
		}

		NGFILE::file_free(pfile);
		NGBASE::core_getsystem()->event_set(param->esharerefresh, false);
	}
	catch(...)
	{
		NGBASE::core_getsystem()->event_set(param->esharerefresh, false);
		return false;
	}
	return true;
}

bool ngwinamp_refreshroots(HNGWINAMP_PARAM param, bool force)
{
	try
	{
		NGWINAMPLOCK lock(param);

		if(param->pvars->vars_getb("sv_roots_enableshares"))
		{
			param->psharetimer->pick();
			if(force || param->psharetimer->s_elapsed >= param->pvars->vars_getf("sv_roots_sharesrefresh"))
			{
				if (!NGBASE::core_getsystem()->event_get(param->esharerefresh, 5))
				{
					param->psharetimer->start();

					// start thread if not running
					NGBASE::core_getsystem()->thread_create(refreshroots_thread, param);
				}
			}
		}
	}
	catch(...)
	{
		return false;
	}
	return true;
}


bool openhelp(HNGWINAMP_PARAM param, HWND hparentwnd)
{
	if (param->hhelpwnd == NULL) {
		param->hhelpwnd = CreateDialog(param->hinstance, MAKEINTRESOURCE(IDD_HELP), hparentwnd, (DLGPROC)helpdlgproc);
	} else {
		SetForegroundWindow(param->hhelpwnd);
	}
	return true;
}

bool openconfig(HNGWINAMP_PARAM param, HWND hparentwnd)
{
	if (param->hconfigwnd == NULL) {
		param->hconfigwnd = CreateDialog(param->hinstance, MAKEINTRESOURCE(IDD_CONFIG), hparentwnd, (DLGPROC)configdlgproc);
	} else {
		SetForegroundWindow(param->hconfigwnd);
	}
	return true;
}

bool netsend(HNGWINAMP_PARAM param, HNETDATA pdata)
{
	if(pdata->s == NGSOCKET_INVALID)
		return false;
	if(pdata->hdr.size > pdata->data.buffer_getlength())
		return false;

	try
	{
		dword bw = 0;

		bw = sizeof(NETHDR);
		if(!NGBASE::core_getsystem()->sock_send(pdata->s, (byte*)&pdata->hdr, &bw, param->equit))
			return false;
		if(pdata->hdr.size > 0)
		{
			bw = pdata->hdr.size;
			if(!NGBASE::core_getsystem()->sock_send(pdata->s, pdata->data.buffer_get(), &bw, param->equit))
				return false;
		}
	}
	catch(...)
	{
		return false;
	}
	return true;
}


bool netcompress(HNGWINAMP_PARAM param, HNETDATA pdata) {
	NGCNTBYTES	tmpbuf;
	dword		out_size = pdata->hdr.size + 32;
	int			code = 0;

	if (pdata->hdr.size != pdata->data.buffer_getlength()) {
		return false;
	}
	tmpbuf.buffer_setlength(out_size);
	tmpbuf.buffer_set(0, pdata->data.buffer_get(), pdata->data.buffer_getlength());
	code = zzip_compress(tmpbuf.buffer_get(), pdata->data.buffer_getlength(), &out_size);
	if(code < 0)
		return false;

	pdata->data.buffer_setlength(out_size);
	pdata->data.buffer_set(0, tmpbuf.buffer_get(), out_size);
	pdata->hdr.reserved1 |= NGWINAMP_FILTER_ZZIP;
	pdata->hdr.reserved2 = pdata->hdr.size;
	pdata->hdr.size = out_size;
	return true;
}

bool netuncompress(HNGWINAMP_PARAM param, HNETDATA pdata) {
	if (pdata->hdr.reserved1 & NGWINAMP_FILTER_ZZIP) {
		NGCNTBYTES	tmpbuf;
		dword		out_size = pdata->hdr.reserved2 + 32;
		int			code = 0;

		if (pdata->hdr.size != pdata->data.buffer_getlength()) {
			return false;
		}
		tmpbuf.buffer_setlength(out_size);
		tmpbuf.buffer_set(0, pdata->data.buffer_get(), pdata->data.buffer_getlength());
		code = zzip_decompress(tmpbuf.buffer_get(), pdata->data.buffer_getlength(), &out_size);
		if(code != 0)
			return false;

		pdata->data.buffer_setlength(out_size);
		pdata->data.buffer_set(0, tmpbuf.buffer_get(), out_size);
		pdata->hdr.reserved1 &= ~(NGWINAMP_FILTER_ZZIP);
		pdata->hdr.reserved2 = 0;
		pdata->hdr.size = out_size;
	}
	return true;
}

void combobox_setheight(HWND hcombo, dword height)
{
	RECT rc;
	GetWindowRect(hcombo, &rc);
	SetWindowPos(hcombo, NULL, 0, 0, rc.right - rc.left, height, SWP_NOZORDER | SWP_NOMOVE);
}


long listview_getsel(HWND hlist)
{
	dword	count = 0;
	long	nbitems = ListView_GetItemCount(hlist);

	for(long i = 0; i < nbitems; i++)
	{
		if(ListView_GetItemState(hlist, i, LVIS_SELECTED) & LVIS_SELECTED)
			return i;
	}
	return -1;
}

void listview_getsels(HWND hlist, NGLONGS* plongs)
{
	register dword count = 0;
	register long  nbitems = ListView_GetItemCount(hlist);

	plongs->realloc(nbitems);
	for(register long i = 0; i < nbitems; i++)
	{
		if(ListView_GetItemState(hlist, i, LVIS_SELECTED) & LVIS_SELECTED)
		{
			plongs->set(count, i);
			count++;
		}
	}
	plongs->realloc(count);
}

LPARAM listview_getlparam(HWND hlist, long index)
{
	LVITEM lvi;

	memset(&lvi, 0, sizeof(LVITEM));
	if(index >= 0)
	{
		lvi.mask = LVIF_PARAM;
		lvi.iItem = index;
		ListView_GetItem(hlist, &lvi);
	}
	return lvi.lParam;
}

long listview_findindex(HWND hlist, LPARAM lparam)
{
	long count = ListView_GetItemCount(hlist);

	for(long i = 0; i < count; i++)
	{
		if(listview_getlparam(hlist, i) == lparam)
			return i;
	}
	return -1;
}

void listview_update(HWND hlist, long index, long subindex, char* text)
{
	LVITEM lvi;
	char   cbuffer[65535];

	ngmemzero(cbuffer, sizeof(cbuffer));
	ngmemzero(&lvi, sizeof(LVITEM));
	lvi.mask = LVIF_TEXT;
	lvi.iItem = index;
	lvi.iSubItem = subindex;
	lvi.pszText = cbuffer;
	lvi.cchTextMax = sizeof(cbuffer);
	ListView_GetItem(hlist, &lvi);
	if(ngstrcmp(cbuffer, text))
	{
		ListView_SetItemText(hlist, index, subindex, text);
	}
}

void listview_setimage(HWND hlist, long index, long image)
{
	LVITEM lvi;

	ngmemzero(&lvi, sizeof(LVITEM));
	lvi.mask = LVIF_IMAGE;
	lvi.iItem = index;
	lvi.iSubItem = 0;
	ListView_GetItem(hlist, &lvi);
	if(lvi.iImage != image)
	{
		lvi.mask = LVIF_IMAGE;
		lvi.iItem = index;
		lvi.iSubItem = 0;
		lvi.iImage = image;
		ListView_SetItem(hlist, &lvi);
	}
}
*/