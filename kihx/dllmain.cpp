// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "memory.h"
#include "render.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	UNREFERENCED_PARAMETER( hModule );
	UNREFERENCED_PARAMETER( lpReserved );

	switch (ul_reason_for_call)
	{
	case DLL_THREAD_DETACH:
	case DLL_THREAD_ATTACH:
		break;

	case DLL_PROCESS_ATTACH:
		kih::InitAllocator();
		break;

	case DLL_PROCESS_DETACH:
		kih::RenderingDevice::DestroyInstance();
		//kih::ShutdownAllocator();
		break;
	}
	return TRUE;
}

