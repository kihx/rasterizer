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
		Matrix4 m_wvpMatrix;

	public:
		CVertexShader() {}
		~CVertexShader() {}

		CRsElementDesc& Process( const std::shared_ptr<CMesh> pMesh );
		
		CRsElementDesc& ProcessParallel( const std::shared_ptr<CMesh> pMesh, CXtzThreadPool* threadPool );

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

		const Matrix4& GetwvpMatrix()
		{
			return m_wvpMatrix;
		}
		Vector4& GetOutputVertex(int index)
		{
			return m_vsOutput.m_vertices[index];
		}
		Vector3& GetOutputLocalVertex( int index )
		{
			return m_vsOutput.m_localVertices[index];
		}
	};

	struct VsThreadArg
	{
		CVertexShader* pVs;
		int index;
		Matrix4 matrix;
		std::shared_ptr<CMesh> pMesh;
	};

	static void VsThreadWork( LPVOID arg );
}
#endif

