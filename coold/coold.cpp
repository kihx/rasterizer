#define DLLEXPORTS
//#include "Windows.h"
//#include <memory>
#include "coold.h"
#include "Data\CoolD_Defines.h"
#include "Data\CoolD_Inlines.h"
#include "Render\CoolD_AreaFilling.h"
#include "Render\CoolD_Transform.h"
#include "Render\CoolD_MeshManager.h"
#include "Render\CoolD_CustomMesh.h"
#include "Render\CoolD_SubFunction.h"
#include "Math\CoolD_Matrix33.h"
#include "Math\CoolD_Matrix44.h"
#include "Math\CoolD_Vector3.h"

using namespace CoolD;

Matrix44 g_matWorld;
Matrix44 g_matView;
Matrix44 g_matPers;

//Dbool WINAPI DllMain( HINSTANCE hInstDll, DWORD fdwReason, LPVOID fImpLoad )
//{
//	return true;
//}

EXTERN_FORM DLL_API Dvoid __cdecl coold_LoadMeshFromFile( const Dchar* filename )
{
	GETSINGLE(MeshManager).Clear();
	GETSINGLE(MeshManager).LoadMesh(filename);
}

EXTERN_FORM DLL_API Dvoid __cdecl coold_RenderToBuffer(Dvoid* buffer, Dint width, Dint height, Dint bpp)
{
	ClearColorBuffer(buffer, width, height, BLACK);
	AreaFilling renderCore;

	parallel_invoke( 
		[&renderCore, buffer, width, height]{ renderCore.SetScreenInfo(buffer, width, height); },
		[&renderCore]{ renderCore.SetTransform(WORLD, g_matWorld); },
		[&renderCore]{ renderCore.SetTransform(VIEW, g_matView); },
		[&renderCore]{ renderCore.SetTransform(PERSPECTIVE, g_matPers); },
		[&renderCore, width, height]{ renderCore.SetTransform(VIEWPORT, TransformHelper::CreateViewport(0, 0, width, height)); }
		);
		
	//병렬 루프 가능-----------------------------------
	/*vector<tuple_meshInfo> vecMeshInfo;
	for( auto& Mesh : GETSINGLE(MeshManager).GetMeshMap() )
	{
		vecMeshInfo.push_back( GETSINGLE(MeshManager).AdjustTransform(Mesh.second, renderCore.GetArrayTransform()) );
	}*/

	vector<tuple_meshInfo> vecMeshInfo;
	//for( auto& Mesh : GETSINGLE(MeshManager).GetMeshMap() )
	const map<string, CustomMesh*>& rMeshMap = GETSINGLE(MeshManager).GetMeshMap();
	parallel_for_each(begin(rMeshMap), end(rMeshMap), [&renderCore, &vecMeshInfo](pair<string, CustomMesh*> pMesh)
	{
		vecMeshInfo.push_back( GETSINGLE(MeshManager).AdjustTransform(pMesh.second, renderCore.GetArrayTransform()) );
	});
	//----------------------------------------------

	for (auto& Mesh : vecMeshInfo)
	{
		renderCore.Render( Mesh );
	}
}
EXTERN_FORM DLL_API Dvoid __cdecl coold_SetTransform(Dint transformType, const Dfloat* matrix4x4)
{	//월드만 넘겨 받는다.
	switch( transformType )
	{
	case 0:	//World
		{
			Matrix44 matWorld(matrix4x4);								
			g_matWorld = matWorld.Transpose();
		}
		break;
	default:
		break;
	}
}

EXTERN_FORM DLL_API Dvoid __cdecl coold_SetViewFactor(Dfloat* eye, Dfloat* lookat, Dfloat* up)
{			
	g_matView = TransformHelper::CreateView(Vector3(eye), Vector3(lookat), Vector3(up));
}

EXTERN_FORM DLL_API Dvoid __cdecl coold_SetPerspectiveFactor(Dfloat fovY, Dfloat aspect, Dfloat zn, Dfloat zf)
{	
	g_matPers = TransformHelper::CreatePerspective(kPI / fovY, aspect, zn, zf);
}