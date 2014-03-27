#include "stdafx.h"
#include "VertexShader.h"

namespace xtozero
{
	std::vector<CRsElementDesc> CVertexShader::Process( const std::shared_ptr<CMesh> pMesh )
	{
		std::vector<CRsElementDesc> vsOutput;
		std::vector<Vector3> vertices;

		vsOutput.reserve( pMesh->m_nVerties * 3 );
		vertices.reserve( pMesh->m_nVerties );

		Matrix4 wvpMatrix = m_worldMatrix * m_viewMatrix * m_projectionMatrix;

		for ( int i = 0; i < pMesh->m_nVerties; ++i )
		{
			Vector3 position = pMesh->m_vertices[i];
			position.Transform( wvpMatrix );

			vertices.emplace_back( position );
		}
		
		for ( std::vector<Face>::iterator faceiter = pMesh->m_faces.begin( ); faceiter != pMesh->m_faces.end( ); ++faceiter )
		{
			for ( std::vector<int>::iterator indexiter = faceiter->m_indices.begin( ); indexiter != faceiter->m_indices.end( ); ++indexiter )
			{
				vsOutput.emplace_back( vertices[*indexiter], faceiter->m_color[r], faceiter->m_color[g], faceiter->m_color[b] );
			}
			
		}

		return vsOutput;
	}
}
