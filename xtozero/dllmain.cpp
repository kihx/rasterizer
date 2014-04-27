// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "Concommand.h"

BOOL APIENTRY DllMain( HMODULE,
	DWORD  ul_reason_for_call,
	LPVOID
	)
{
	switch ( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		cmd::CConcommandExecutor::GetInstance()->ReleaseInstance();
		break;
	}
	return TRUE;
}

