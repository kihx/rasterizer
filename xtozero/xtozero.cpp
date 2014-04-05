// xtozero.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "xtozero.h"

using namespace xtozero;

std::unique_ptr<xtozero::CMeshManager> gMeshManager( new xtozero::CMeshManager( ) );
std::unique_ptr<xtozero::CVertexShader> gVertexShader( new xtozero::CVertexShader( ) );
std::unique_ptr<xtozero::CRasterizer> gRasterizer( new xtozero::CRasterizer( ) );
std::unique_ptr<xtozero::CPixelShader> gPixelShader( new xtozero::CPixelShader() );
std::unique_ptr<xtozero::COutputMerger> gOutputMerger( new xtozero::COutputMerger() );

XTZ_API void XtzRenderToBuffer( void* buffer, int width, int height, int dpp )
{
	if ( buffer )
	{
		gRasterizer->SetViewPort( 0, 0, width, height );
		gOutputMerger->SetFrameBuffer( buffer, dpp, width, height );
		CRsElementDesc& vsOut = gVertexShader->Process( gMeshManager->LoadRecentMesh() );
		std::vector<CPsElementDesc>& rsOut = gRasterizer->Process( vsOut );
		std::vector<COmElementDesc>& psOut = gPixelShader->Process( rsOut );
		gOutputMerger->Process( psOut );
	}
}

XTZ_API void XtzClearBuffer( void* buffer, int width, int height, int color )
{

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