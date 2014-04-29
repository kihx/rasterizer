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
#include "Math\CoolD_Matrix33.h"
#include "Math\CoolD_Matrix44.h"
#include "Math\CoolD_Vector3.h"
#include "Console\CoolD_Command.h"
#include "Console\CoolD_ConsoleManager.h"

#pragma warning(disable: 4100)
using namespace CoolD;

Matrix44 g_matWorld;
Matrix44 g_matView;
Matrix44 g_matPers;

//Dbool WINAPI DllMain( HINSTANCE hInstDll, DWORD fdwReason, LPVOID fImpLoad )
//{
//	return true;
//}

static VariableCommand test("test", "0");
static FunctionCommand testfunc("testfunc", [](initializer_list<string> strs)
{ 	
	//함수 인자를 쓰려면 함수 안에서 적절하게 형변환 해서 써야한다.
	for( auto& it : strs )
	{
		string d = it;
	}
});

static FunctionCommand ShowCommand("show_command", [] (initializer_list<string> commandNameList)
{
	for( auto& commandName : commandNameList )
	{
		GETSINGLE(ConsoleManager).ShowCommand(commandName);
	}
});

auto renderCore = make_unique<AreaFilling>();
EXTERN_FORM DLL_API Dvoid __cdecl coold_LoadMeshFromFile( const Dchar* filename )
{
	GETSINGLE(MeshManager).Clear();
	GETSINGLE(MeshManager).LoadMesh(filename);
}

EXTERN_FORM DLL_API Dvoid __cdecl coold_RenderToBuffer(Dvoid* buffer, Dint width, Dint height, Dint bpp)
{	
	renderCore->SetScreenInfo(buffer, width, height);
	renderCore->ClearColorBuffer(BLACK);
	renderCore->SetTransform(WORLD, g_matWorld);
	renderCore->SetTransform(VIEW, g_matView);
	renderCore->SetTransform(PERSPECTIVE, g_matPers);
	renderCore->SetTransform(VIEWPORT, TransformHelper::CreateViewport(0, 0, width, height));		

	for( auto& Mesh : GETSINGLE(MeshManager).GetMapMesh() )
	{
		GETSINGLE(MeshManager).AdjustTransform(Mesh.second, renderCore->GetArrayTransform());	
		renderCore->Render( GETSINGLE(MeshManager).GetVecTransformVertex(), Mesh.second->GetVectorFace());
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
								  
EXTERN_FORM DLL_API Dvoid __cdecl coold_ExecuteCommand(const Dchar* cmd)
{
	GETSINGLE(ConsoleManager).CommandExecute(cmd);
}