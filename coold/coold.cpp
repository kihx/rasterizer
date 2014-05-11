#define DLLEXPORTS
//#include "Windows.h"
//#include <memory>
#include "coold.h"
#include "Data\CoolD_Defines.h"
#include "Data\CoolD_Inlines.h"
#include "Render\CoolD_RenderModule.h"
#include "Render\CoolD_Transform.h"
#include "Render\CoolD_MeshManager.h"
#include "Render\CoolD_CustomMesh.h"
#include "Math\CoolD_Matrix33.h"
#include "Math\CoolD_Matrix44.h"
#include "Math\CoolD_Vector3.h"
#include "Console\CoolD_Command.h"
#include "Console\CoolD_ConsoleManager.h"
#include "Thread\CoolD_ThreadManager.h"

#pragma warning(disable: 4100)

using namespace CoolD;

Matrix44 g_matWorld;
Matrix44 g_matView;
Matrix44 g_matPers;

//Dbool WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID fImpLoad)
//{
//	switch( fdwReason )
//	{
//	case DLL_THREAD_ATTACH:
//	case DLL_THREAD_DETACH:		
//	case DLL_PROCESS_ATTACH:
//	case DLL_PROCESS_DETACH:
//		break;
//	}	
//	return true;
//}

static FunctionCommand ShowCommand("show_command", [] (initializer_list<string> commandNameList)
{
	for( auto& commandName : commandNameList )
	{
		GETSINGLE(ConsoleManager).ShowCommand(commandName);
	}
});

auto g_renderModule = new RenderModule();
static VariableCommand cc_mesh_copy("cc_mesh_copy", "0");
EXTERN_FORM DLL_API Dvoid __cdecl coold_LoadMeshFromFile( const Dchar* filename )
{
	GETSINGLE(MeshManager).Clear();		
	GETSINGLE(MeshManager).LoadMesh(filename);	

	//쓰레드 테스트를 위한 추가 로직----------------------------------
	for (Dint i = 0; i < cc_mesh_copy.Integer(); ++i)
	{
		CustomMesh* pMesh = GETSINGLE(MeshManager).GetMesh(filename);
		if (CustomMeshPLY* pPlyMesh = dynamic_cast<CustomMeshPLY*>(pMesh))
		{
			CustomMesh* pNewMesh = new CustomMeshPLY(*pPlyMesh);
			GETSINGLE(MeshManager).AddMesh(filename + to_string(i), pNewMesh);
		}		
	}
	//-----------------------------------------------------------
}

Dbool	g_threadUse = true;
ThreadManager* g_pThreadManager = &GETSINGLE(ThreadManager);
EXTERN_FORM DLL_API Dvoid __cdecl coold_RenderToBuffer(Dvoid* buffer, Dint width, Dint height, Dint bpp)
{	
	g_renderModule->SetScreenInfo(buffer, width, height);
	g_renderModule->ClearColorBuffer(BLACK);
	g_renderModule->SetTransform(WORLD, g_matWorld);
	g_renderModule->SetTransform(VIEW, g_matView);
	g_renderModule->SetTransform(PERSPECTIVE, g_matPers);
	g_renderModule->SetTransform(VIEWPORT, TransformHelper::CreateViewport(0, 0, width, height));

	for( auto& Mesh : GETSINGLE(MeshManager).GetMapMesh() )
	{
		if( g_threadUse )
		{
			RenderInfoParam param( g_renderModule, Mesh.second );
			g_pThreadManager->AssignWork(&param);
		}
		else
		{
			g_renderModule->RenderBegin(Mesh.second);
			g_renderModule->RenderEnd();
		}
	}
	
	g_pThreadManager->WaitAllThreadWorking();
}

static VariableCommand cc_x_move("cc_x_move", "0");
static VariableCommand cc_y_move("cc_y_move", "0");
static VariableCommand cc_z_move("cc_z_move", "0");
EXTERN_FORM DLL_API Dvoid __cdecl coold_SetTransform(Dint transformType, const Dfloat* matrix4x4)
{	//월드만 넘겨 받는다.
	switch( transformType )
	{
	case 0:	//World
		{
			Matrix44 matWorld(matrix4x4);								
			g_matWorld = matWorld.Transpose();

			//원점 기준으로 이동
			if( cc_x_move.Bool() || cc_y_move.Bool() || cc_z_move.Bool() )
			{
				g_matWorld[ 12 ] = cc_x_move.Float();
				g_matWorld[ 13 ] = cc_y_move.Float();
				g_matWorld[ 14 ] = cc_z_move.Float();
			}					
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

static VariableCommand cc_adjust_zn("cc_adjust_zn", "0");
static VariableCommand cc_adjust_zf("cc_adjust_zf", "0");
EXTERN_FORM DLL_API Dvoid __cdecl coold_SetPerspectiveFactor(Dfloat fovY, Dfloat aspect, Dfloat zn, Dfloat zf)
{	
	g_matPers = TransformHelper::CreatePerspective(kPI / fovY, aspect, zn + cc_adjust_zn.Float(), zf + cc_adjust_zf.Float());
}
								  
EXTERN_FORM DLL_API Dvoid __cdecl coold_ExecuteCommand(const Dchar* cmd)
{
	GETSINGLE(ConsoleManager).CommandExecute(cmd);
}

EXTERN_FORM DLL_API Dvoid __cdecl coold_DetachModuleClear()
{
	GETSINGLE(ThreadManager).CleanThreads();
}

//-----------------------------------------------------------------------------------------------------------------------------
static FunctionCommand fc_bsculltype("fc_bsculltype", [] (initializer_list<string> strs)
{
	if( strs.size() > 0 )
	{
		Dint type = stoi((*begin(strs)));		//stoi는 숫자문자가 아닌 값이 들어오면 프로그램을 종료시킨다.
		g_renderModule->SetBackSpaceCullType(static_cast<BSCullType>(type));
	}
	else
	{
		LOG("don't input Backspace CullingType ( 0 : CW, 1 : CCW, the other number : ALL )");
	}
});

static FunctionCommand fc_thread_count("fc_thread_count", [](initializer_list<string> strs)
{
	GETSINGLE(ThreadManager).ResetThreads( stoi(*strs.begin()) );
});

static FunctionCommand fc_thread_use("fc_thread_use", [] (initializer_list<string> strs)
{
	if( strs.size() == 1 )
	{
		Dbool isUse;

		if( stoi(*strs.begin()) != 0 )
			isUse = true;
		else
			isUse = false;

		if( g_threadUse != isUse )
		{
			GETSINGLE(ThreadManager).ResetThreads(0);
			g_threadUse = isUse;
		}		
	}
});