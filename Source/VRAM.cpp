
#include "VRAM.h"
#include "glew-2.1.0\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#pragma comment( lib, "glew-2.1.0/lib/glew32.lib")
#pragma comment( lib, "glew-2.1.0/lib/glew32s.lib")

BOOL WINAPI DDEnumCallbackEx(GUID FAR* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm) {
	UNREFERENCED_PARAMETER(lpDriverDescription);

	DDRAW_MATCH* pDDMatch = (DDRAW_MATCH*)lpContext;
	if (pDDMatch->hMonitor == hm) {
		pDDMatch->bFound = true;
		strcpy_s(pDDMatch->strDriverName, 512, lpDriverName);
		memcpy(&pDDMatch->guid, lpGUID, sizeof(GUID));
	}
	return TRUE;
}

HRESULT GetVideoMemoryViaDirectDraw(HMONITOR hMonitor, DWORD* pdwAvailableVidMem) {
	LPDIRECTDRAW pDDraw = nullptr;
	LPDIRECTDRAWENUMERATEEXA pDirectDrawEnumerateEx = nullptr;
	HRESULT hr;
	bool bGotMemory = false;
	*pdwAvailableVidMem = 0;

	HINSTANCE hInstDDraw;
	LPDIRECTDRAWCREATE pDDCreate = nullptr;

	hInstDDraw = LoadLibrary("ddraw.dll");
	if (hInstDDraw) {
		DDRAW_MATCH match;
		ZeroMemory(&match, sizeof(DDRAW_MATCH));
		match.hMonitor = hMonitor;

		pDirectDrawEnumerateEx = (LPDIRECTDRAWENUMERATEEXA)GetProcAddress(hInstDDraw, "DirectDrawEnumerateExA");

		if (pDirectDrawEnumerateEx) {
			hr = pDirectDrawEnumerateEx(DDEnumCallbackEx, (VOID*)&match, DDENUM_ATTACHEDSECONDARYDEVICES);
		}

		pDDCreate = (LPDIRECTDRAWCREATE)GetProcAddress(hInstDDraw, "DirectDrawCreate");
		if (pDDCreate) {
			pDDCreate(&match.guid, &pDDraw, nullptr);

			LPDIRECTDRAW7 pDDraw7;
			if (SUCCEEDED(pDDraw->QueryInterface(IID_IDirectDraw7, (VOID**)&pDDraw7))) {
				DDSCAPS2 ddscaps;
				ZeroMemory(&ddscaps, sizeof(DDSCAPS2));
				ddscaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
				hr = pDDraw7->GetAvailableVidMem(&ddscaps, pdwAvailableVidMem, nullptr);
				if (SUCCEEDED(hr))
					bGotMemory = true;
				pDDraw7->Release();
			}
		}
		FreeLibrary(hInstDDraw);
	}


	if (bGotMemory)
		return S_OK;
	else
		return E_FAIL;
}

float getAvaliableVRAMMb() {
	HMONITOR  monitor;
	DWORD memory;
	monitor = MonitorFromPoint({ 0,0 }, MONITOR_DEFAULTTOPRIMARY);
	

	if (GetVideoMemoryViaDirectDraw(monitor, &memory) == S_OK){
		float float_memory = (float)memory;
		return float_memory /(1000000); // Bytes to MB
	}
	else
		return 0;
}

float getTotalVRAMMb_NVIDIA() {
	GLint total_mem_kb = 0;
	glGetIntegerv(0x9048, &total_mem_kb);
	return total_mem_kb / (1000); // KB to MB
}
float getAvaliableVRAMMb_NVIDIA() {
	GLint avaliable_mem_kb = 0;
	glGetIntegerv(0x9049, &avaliable_mem_kb);
	return avaliable_mem_kb / (1000); // KB to MB
}


