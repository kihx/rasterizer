#ifndef _VERTEXSHADER_H_
#define _VERTEXSHADER_H_

#include "Mesh.h"
#include "pipelineElements.h"

#include <vector>

namespace xtozero
{
	class CVertexShader
	{
	private:
		Matrix4 m_worldMatrix;
		Matrix4 m_viewMatrix;
		Matrix4 m_projectionMatrix;
	public:
		CVertexShader();
		~CVertexShader();

		std::vector<CRsElementDesc> Process( const std::shared_ptr<CMesh> pMesh );
	};
}
#endif

