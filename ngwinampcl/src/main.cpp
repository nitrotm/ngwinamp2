// main.cpp
#include <windows.h>


typedef void (*HCLIENTMAINFUNC)(void);




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR Parameters, int ShowValue)
{
	HCLIENTMAINFUNC pfunc = NULL;
	HMODULE			hModule = LoadLibrary("gen_ngwinamp.dll");


	if(hModule == NULL)
	{
		MessageBox(NULL, "Cannot find file !", "gen_ngwinamp.dll", MB_ICONERROR);
		return 0;
	}

	if(lstrlen(Parameters) > 0)
	{
		if(lstrcmp(Parameters, "-admin") == 0)
			pfunc = (HCLIENTMAINFUNC)GetProcAddress(hModule, "createClientAdmin");
	}
	else
		pfunc = (HCLIENTMAINFUNC)GetProcAddress(hModule, "createClientUser");

	if(pfunc != NULL)
	{
		try
		{
			pfunc();
		}
		catch(...)
		{
			MessageBox(NULL, "Error during call to entry-point !", "gen_ngwinamp.dll", MB_ICONERROR);
		}
	}
	else
		MessageBox(NULL, "Unable to find entry-point !", "gen_ngwinamp.dll", MB_ICONERROR);

	FreeLibrary(hModule);
	return 0;
}
