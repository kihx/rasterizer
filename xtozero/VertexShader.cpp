#include "stdafx.h"
#include "VertexShader.h"

namespace xtozero
{
	CRsElementDesc CVertexShader::Process( const std::shared_ptr<CMesh>& pMesh )
	{
		CRsElementDesc vsOutput;

		vsOutput.m_vertices.reserve( pMesh->m_nVerties );

		Matrix4 wvpMatrix = m_worldMatrix * m_viewMatrix * m_projectionMatrix;

		for ( int i = 0; i < pMesh->m_nVerties; ++i )
		{
			Vector3 position = pMesh->m_vertices[i];
			position.Transform( wvpMatrix );

			vsOutput.m_vertices.emplace_back( position );
		}
		
		for ( std::vector<Face>::iterator faceiter = pMesh->m_faces.begin( ); faceiter != pMesh->m_faces.end( ); ++faceiter )
		{
			int key = faceiter - pMesh->m_faces.begin();
			if ( vsOutput.m_faces.find( key ) == vsOutput.m_faces.end( ) )
			{
				vsOutput.m_faces[key] = std::vector<int>();
			}
			for ( std::vector<int>::iterator indexiter = faceiter->m_indices.begin( ); indexiter != faceiter->m_indices.end( ); ++indexiter )
			{
				vsOutput.m_faces[key].emplace_back( *indexiter );
			}
		}

		return vsOutput;
	}
}
