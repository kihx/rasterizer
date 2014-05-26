// xtozero.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "xtozero.h"
#include "Concommand.h"

#include "Mesh.h"
#include "Texture.h"
#include "Rasterizer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "OutputMerger.h"
#include "XtzThreadPool.h"
#include "Concommand.h"


using namespace xtozero;

std::unique_ptr<xtozero::CXtzThreadPool> gThreadPool( new xtozero::CXtzThreadPool( ) );
std::unique_ptr<xtozero::CMeshManager> gMeshManager( new xtozero::CMeshManager( ) );
std::unique_ptr<xtozero::CTextureManager> gTextureManager( new xtozero::CTextureManager( ) );
std::unique_ptr<xtozero::CVertexShader> gVertexShader( new xtozero::CVertexShader( ) );
std::unique_ptr<xtozero::CRasterizer> gRasterizer( new xtozero::CRasterizer( ) );
std::unique_ptr<xtozero::CPixelShader> gPixelShader( new xtozero::CPixelShader() );
std::unique_ptr<xtozero::COutputMerger> gOutputMerger( new xtozero::COutputMerger() );
std::unique_ptr<xtozero::CBarycentricRasterizer> gBarycentricRasterizer( new xtozero::CBarycentricRasterizer( ) );

DECLARE_CONCOMMAND( SetThread )
{
	if ( cmd::CConcommandExecutor::GetInstance()->ArgC() > 1 )
	{
		std::string threadNumber = cmd::CConcommandExecutor::GetInstance()->ArgV( 1 );
		gThreadPool->CreateThreadPool( atoi( threadNumber.c_str( ) ) );
	}
}

DECLARE_CONCOMMAND( LoadTexture )
{
	if ( cmd::CConcommandExecutor::GetInstance( )->ArgC( ) > 1 )
	{
		std::string fileName = cmd::CConcommandExecutor::GetInstance( )->ArgV( 1 );
		std::shared_ptr<xtozero::CTexture> texture = gTextureManager->Load( fileName.c_str( ) );

		if ( texture )
		{
			std::cout << "텍스쳐 로드 성공" << std::endl;
		}
		else
		{
			std::cout << "텍스쳐 로드 실패" << std::endl;
		}
	}
}

void TestThreadFunc( LPVOID arg )
{
	std::cout << (int)arg << std::endl;
}

struct RendererThreadArg
{
	CRsElementDesc* rsInput;
	COutputMerger*	pOm;
	unsigned int	startIdx;
	unsigned int	endIdx;
	Rect			viewport;
};

void RendererThreadWork( LPVOID arg )
{
	RendererThreadArg*		pArg = static_cast<RendererThreadArg*>(arg);

	std::shared_ptr<xtozero::CTexture> texture = gTextureManager->Load( "texture.bmp" );

	xtozero::CRasterizer	rs;
	xtozero::CPixelShader	ps;
	COutputMerger*			om = pArg->pOm;

	const Rect& vp = pArg->viewport;
	ps.PSSetTexture( 0, texture.get() );

	//Rasterizer
	rs.SetViewPort( vp.m_left, vp.m_top, vp.m_right, vp.m_bottom );
	const std::vector<CPsElementDesc>& rsOut = rs.ProcessFaceRange( *pArg->rsInput, pArg->startIdx, pArg->endIdx );

	//PixelShader
	const std::vector<COmElementDesc>& psOut = ps.Process( rsOut );

	//OutputMerger
	om->ProcessParallel( psOut );

	delete pArg;
}

void RendererThreadWorkByBarycentric( LPVOID arg )
{
	RendererThreadArg*		pArg = static_cast<RendererThreadArg*>(arg);

	std::shared_ptr<xtozero::CTexture> texture = gTextureManager->Load( "texture.bmp" );

	xtozero::CBarycentricRasterizer	rs;
	xtozero::CPixelShader			ps;
	COutputMerger*					om = pArg->pOm;

	const Rect& vp = pArg->viewport;
	ps.PSSetTexture( 0, texture.get( ) );

	//Rasterizer
	rs.SetViewPort( vp.m_left, vp.m_top, vp.m_right, vp.m_bottom );
	const std::vector<CPsElementDesc>& rsOut = rs.ProcessFaceRange( *pArg->rsInput, pArg->startIdx, pArg->endIdx );

	//PixelShader
	const std::vector<COmElementDesc>& psOut = ps.Process( rsOut );

	//OutputMerger
	om->ProcessParallel( psOut );

	delete pArg;
}

XTZ_API void XtzRenderToBuffer( void* buffer, int width, int height, int dpp )
{
	if ( buffer )
	{
		//gRasterizer->SetViewPort( 0, 0, width, height );
		gBarycentricRasterizer->SetViewPort( 0, 0, width, height );
		gOutputMerger->CreateDepthBuffer( width, height );
		gOutputMerger->ClearDepthBuffer();
		gOutputMerger->SetFrameBuffer( buffer, dpp, width, height );
		CRsElementDesc& vsOut = gVertexShader->ProcessParallel( gMeshManager->LoadRecentMesh( ), gThreadPool.get( ) );
		//const std::vector<CPsElementDesc>& rsOut = gRasterizer->ProcessParallel( vsOut, gThreadPool.get() );
		const std::vector<CPsElementDesc>& rsOut = gBarycentricRasterizer->Process( vsOut );
		//CRsElementDesc& vsOut = gVertexShader->Process( gMeshManager->LoadRecentMesh() );
		//const std::vector<CPsElementDesc>& rsOut = gRasterizer->Process( vsOut );
		const std::vector<COmElementDesc>& psOut = gPixelShader->Process( rsOut );
		gOutputMerger->Process( psOut );
	}
}

XTZ_API void XtzRenderToBuffer3StageParallel( void* buffer, int width, int height, int dpp )
{
	if ( buffer )
	{
		gOutputMerger->CreateDepthBuffer( width, height );
		gOutputMerger->ClearDepthBuffer( );
		gOutputMerger->SetFrameBuffer( buffer, dpp, width, height );
		gOutputMerger->ClearFrameBuffer( );
		CRsElementDesc& vsOut = gVertexShader->ProcessParallel( gMeshManager->LoadRecentMesh( ), gThreadPool.get( ) );
		
		if ( vsOut.m_coordinate == COORDINATE::OBJECT_COORDINATE )
		{
			for ( std::vector<Vector4>::iterator iter = vsOut.m_vertices.begin( ); iter != vsOut.m_vertices.end( ); ++iter )
			{
				Vector4& vertex = *iter;
				vertex /= vertex.W;

				vertex.X = (vertex.X * width * 0.5f) + width * 0.5f;
				vertex.Y = -(vertex.Y * height * 0.5f) + height * 0.5f;
			}
		}

		int threadFace = static_cast<int>(vsOut.m_faces.size( ) / gThreadPool->GetThreadNumber( ));
		int extra = vsOut.m_faces.size( ) % gThreadPool->GetThreadNumber( );
		if ( extra != 0 )
		{
			threadFace++;
		}

		for ( unsigned int i = 0; i < gThreadPool->GetThreadNumber( ); ++i )
		{
			RendererThreadArg* pRthreadArg = new RendererThreadArg;

			pRthreadArg->rsInput = &vsOut;
			pRthreadArg->pOm = gOutputMerger.get();
			pRthreadArg->startIdx = i * threadFace;
			pRthreadArg->endIdx = pRthreadArg->startIdx + threadFace;

			if ( pRthreadArg->endIdx > vsOut.m_faces.size( ) )
			{
				pRthreadArg->endIdx = vsOut.m_faces.size( );
			}

			pRthreadArg->viewport.m_left = 0;
			pRthreadArg->viewport.m_right = width;
			pRthreadArg->viewport.m_top = 0;
			pRthreadArg->viewport.m_bottom = height;

			gThreadPool->AddWork( RendererThreadWork, (LPVOID)pRthreadArg );
		}

		gThreadPool->WaitThread();
	}
}

XTZ_API void XtzRenderToBufferBarycentricParallel( void* buffer, int width, int height, int dpp )
{
	if ( buffer )
	{
		gOutputMerger->CreateDepthBuffer( width, height );
		gOutputMerger->ClearDepthBuffer( );
		gOutputMerger->SetFrameBuffer( buffer, dpp, width, height );
		gOutputMerger->ClearFrameBuffer( );
		CRsElementDesc& vsOut = gVertexShader->ProcessParallel( gMeshManager->LoadRecentMesh( ), gThreadPool.get( ) );

		if ( vsOut.m_coordinate == COORDINATE::OBJECT_COORDINATE )
		{
			for ( std::vector<Vector4>::iterator iter = vsOut.m_vertices.begin( ); iter != vsOut.m_vertices.end( ); ++iter )
			{
				Vector4& vertex = *iter;
				vertex /= vertex.W;

				vertex.X = (vertex.X * width * 0.5f) + width * 0.5f;
				vertex.Y = -(vertex.Y * height * 0.5f) + height * 0.5f;
			}
		}

		int threadFace = static_cast<int>(vsOut.m_faces.size( ) / gThreadPool->GetThreadNumber( ));
		int extra = vsOut.m_faces.size( ) % gThreadPool->GetThreadNumber( );
		if ( extra != 0 )
		{
			threadFace++;
		}

		for ( unsigned int i = 0; i < gThreadPool->GetThreadNumber( ); ++i )
		{
			RendererThreadArg* pRthreadArg = new RendererThreadArg;

			pRthreadArg->rsInput = &vsOut;
			pRthreadArg->pOm = gOutputMerger.get( );
			pRthreadArg->startIdx = i * threadFace;
			pRthreadArg->endIdx = pRthreadArg->startIdx + threadFace;

			if ( pRthreadArg->endIdx > vsOut.m_faces.size( ) )
			{
				pRthreadArg->endIdx = vsOut.m_faces.size( );
			}

			pRthreadArg->viewport.m_left = 0;
			pRthreadArg->viewport.m_right = width;
			pRthreadArg->viewport.m_top = 0;
			pRthreadArg->viewport.m_bottom = height;

			gThreadPool->AddWork( RendererThreadWorkByBarycentric, (LPVOID)pRthreadArg );
		}

		gThreadPool->WaitThread( );
	}
}


XTZ_API void XtzLoadMeshFromFile( const char* pfilename )
{
	gMeshManager->LoadMeshFromFile( pfilename );
}

XTZ_API void XtzSetTransform(int transformType, const float* matrix4x4)
{
	switch ( transformType )
	{
	case 0:
		gVertexShader->SetWorldMatrix( matrix4x4 );
		break;
	case 1:
		gVertexShader->SetViewMatrix( matrix4x4 );
		break;
	case 2:
		gVertexShader->SetProjectionMatrix( matrix4x4 );
		break;
	}
}

XTZ_API void XtzExecuteCommand( const char* cmd )
{
	cmd::CConcommandExecutor::GetInstance()->DoTokenizing( cmd );
	cmd::CConcommandExecutor::GetInstance()->ExcuteConcommand( );
}