#ifndef _VERTEXSHADER_H_
#define _VERTEXSHADER_H_

#include "Mesh.h"
#include "pipelineElements.h"
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
}
#endif

