#define DLLEXPORTS
//#include "Windows.h"
#include "coold.h"
#include "CoolD_CustomMesh.h"
#include "CoolD_SubFunction.h"
#include "CoolD_Defines.h"

//BOOL WINAPI DllMain( HINSTANCE hInstDll, DWORD fdwReason, LPVOID fImpLoad )
//{
//	return TRUE;
//}

EXTERN_FORM DLL_API void __cdecl coold_LoadMeshFromFile( const char* filename )
{
	CoolD::CustomMesh::CreateMeshFromFile( filename );
}

EXTERN_FORM DLL_API void __cdecl coold_RenderToBuffer( void* buffer, int width, int height, int bpp )
{
	ClearColorBuffer(buffer, width, height, RED );
}

