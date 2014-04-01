#define DLLEXPORTS
//#include "Windows.h"
//#include <memory>
#include "coold.h"
#include "CoolD_CustomMesh.h"
#include "CoolD_SubFunction.h"
#include "CoolD_Defines.h"
#include "CoolD_Inlines.h"
#include "CoolD_AreaFilling.h"
#include "CoolD_Transform.h"
#include "CoolD_Matrix33.h"
#include "CoolD_Matrix44.h"
#include "CoolD_Vector3.h"
#include "CoolD_MeshManager.h"

using namespace CoolD;

//Dbool WINAPI DllMain( HINSTANCE hInstDll, DWORD fdwReason, LPVOID fImpLoad )
//{
//	return true;
//}

EXTERN_FORM DLL_API Dvoid __cdecl coold_LoadMeshFromFile( const Dchar* filename )
{
	GETSINGLE(MeshManager).Clear();
	GETSINGLE(MeshManager).LoadMesh(filename);
}

EXTERN_FORM DLL_API Dvoid __cdecl coold_RenderToBuffer( Dvoid* buffer, Dint width, Dint height, Dint bpp )
{
	ClearColorBuffer(buffer, width, height, BLACK );

	GETSINGLE(TransformHandler).CreateViewport(0, 0, width, height);
	
	CoolD::AreaFilling areaFilling(buffer, width, height);		

	const list<CustomMesh*>& RenderList = GETSINGLE(MeshManager).AdjustTransform();
	
	for(auto& pMesh : RenderList)
	{
		areaFilling.Render( pMesh );
	}	
}

EXTERN_FORM DLL_API Dvoid __cdecl coold_SetTransform(Dint transformType, const Dfloat* matrix4x4)
{	//넘겨 받은 값을 사용하지 않는다. 다만 program.cpp에 있는 값을 동일하게 적용한다.
	switch( transformType )
	{
	case 0:	//World
		{
				Matrix44 matWorld;
				Matrix44 matScale;
				matScale.Scaling(10.0f, 10.0f, 10.0f);
				matWorld = matScale;
				GETSINGLE(TransformHandler).SetTransform(WORLD, matWorld);
		}
		break;
	case 1:	//View
		{
				Vector3 vEyePt(1.5f, 3.0f, -5.0f);
				Vector3 vLookatPt(0.0f, 0.0f, 0.0f);
				Vector3 vUpVec(0.0f, 1.0f, 0.0f);				
				GETSINGLE(TransformHandler).CreateView(vEyePt, vLookatPt, vUpVec);
		}
		break;
	case 2:	//Projection
		{				
				GETSINGLE(TransformHandler).CreatePerspective(kPI / 4.0f, 1.0f, 1.0f, 100.0f);
		}
		break;
	}
}