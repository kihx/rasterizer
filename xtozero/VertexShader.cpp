#include "stdafx.h"
#include "VertexShader.h"

namespace xtozero
{
	CRsElementDesc CVertexShader::Process( const std::shared_ptr<CMesh> pMesh )
	{
		CRsElementDesc vsOutput;

		vsOutput.m_vertices.reserve( pMesh->m_nVerties );

		Matrix4 wvpMatrix = m_worldMatrix * m_viewMatrix * m_projectionMatrix;

		for ( int i = 0; i < pMesh->m_nVerties; ++i )
		{
			Vector3 position = pMesh->m_vertices[i];
			if ( pMesh->m_coordinate == COORDINATE::OBJECT_COORDINATE )
			{
				position.Transform( wvpMatrix );
			}

			vsOutput.m_vertices.emplace_back( position.X, position.Y, position.Z );
		}
		
		int key = 0;
		for ( std::vector<Face>::iterator& faceiter = pMesh->m_faces.begin( ); faceiter != pMesh->m_faces.end( ); ++faceiter )
		{
			key = faceiter - pMesh->m_faces.begin( );
			if ( vsOutput.m_faces.size( ) <= key )
			{
				vsOutput.m_faces.emplace_back();
				vsOutput.m_faces[key].reserve( pMesh->m_nfaces );
			}
			for ( std::vector<int>::iterator& indexiter = faceiter->m_indices.begin( ); indexiter != faceiter->m_indices.end( ); ++indexiter )
			{
				vsOutput.m_faces[key].emplace_back( *indexiter );
			}
		}

		vsOutput.m_coodinate = pMesh->m_coordinate;

		return vsOutput;
	}
}
