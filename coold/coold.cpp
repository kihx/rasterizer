#define DLLEXPORTS
//#include "Windows.h"
//#include <memory>
#include "coold.h"
#include "CoolD_CustomMesh.h"
#include "CoolD_SubFunction.h"
#include "CoolD_Defines.h"
#include "CoolD_Inlines.h"
#include "CoolD_AreaFilling.h"


//BOOL WINAPI DllMain( HINSTANCE hInstDll, DWORD fdwReason, LPVOID fImpLoad )
//{
//	return TRUE;
//}

list<Line*> listLine;
map<Dint, list<Line*>> chainTable;
map<LineKey, EdgeNode*> edgeTable;

CoolD::CustomMesh* g_pMesh = nullptr;
EXTERN_FORM DLL_API void __cdecl coold_LoadMeshFromFile( const Dchar* filename )
{
	if( g_pMesh )
	{
		Safe_Delete( g_pMesh );
	}
	
	g_pMesh = CoolD::CustomMesh::CreateMeshFromFile( filename );
}

EXTERN_FORM DLL_API void __cdecl coold_RenderToBuffer( void* buffer, Dint width, Dint height, Dint bpp )
{
	ClearColorBuffer(buffer, width, height, BLACK );

	CoolD::AreaFilling areaFilling(buffer, width, height);
	areaFilling.AddMesh(g_pMesh);
	areaFilling.Render();
}

