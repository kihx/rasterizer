#ifndef _VERTEXSHADER_H_
#define _VERTEXSHADER_H_

#include "Mesh.h"
#include "pipelineElements.h"
#include "XtzThreadPool.h"
#include <utility>

#include <vector>

namespace xtozero
{
	class CVertexShader
	{
	private:
		CRsElementDesc m_vsOutput;
		Matrix4 m_worldMatrix;
		Matrix4 m_viewMatrix;
		Matrix4 m_projectionMatrix;
	public:
		CVertexShader() {}
		~CVertexShader() {}

		CRsElementDesc& Process( const std::shared_ptr<CMesh> pMesh );
		
		CRsElementDesc& ProcessParallel( const std::shared_ptr<CMesh> pMesh, std::shared_ptr<xtozero::CXtzThreadPool> threadPool );
		void InsertTransformedVertex( Vector3& pos, int index );

		void SetWorldMatrix( const float* mmatrix4x4 )
		{
			memcpy( &m_worldMatrix, mmatrix4x4, sizeof(Matrix4) );
		}
		void SetViewMatrix( const float* mmatrix4x4 )
		{
			memcpy( &m_viewMatrix, mmatrix4x4, sizeof(Matrix4) );
		}
		void SetProjectionMatrix( const float* mmatrix4x4 )
		{
			memcpy( &m_projectionMatrix, mmatrix4x4, sizeof(Matrix4) );
		}
	};

	struct VsThreadArg
	{
		CVertexShader* pVs;
		int index;
		Matrix4 matrix;
		std::shared_ptr<CMesh> pMesh;
	};

	static void VsThreadWork(LPVOID arg)
	{
		VsThreadArg* pVsarg = (VsThreadArg*)arg;
		
		Vector3 position = pVsarg->pMesh->m_vertices[pVsarg->index];
		if ( pVsarg->pMesh->m_coordinate == COORDINATE::OBJECT_COORDINATE )
		{
			position.Transform( pVsarg->matrix );
		}

		pVsarg->pVs->InsertTransformedVertex( position, pVsarg->index );

		delete pVsarg;
	}
}
#endif

