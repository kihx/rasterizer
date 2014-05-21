#include "stdafx.h"
#include "VertexShader.h"

#include <cmath>

namespace xtozero
{
	const float INV_PI = 1 / 3.1415926535897f;

	Vector2 CalcShpericalTexCoord( const Vector4& vertex )
	{
		//Local 좌표에서 UV 값을 계산.

		Vector4 normalizeVertex = vertex;

		normalizeVertex.Normalize();

		//http://en.wikipedia.org/wiki/UV_mapping#Finding_UV_on_a_sphere
		return Vector2( 0.5f + (atan2( normalizeVertex.Z, normalizeVertex.X ) * 0.5f * INV_PI)
			, 0.5f - (asin( normalizeVertex.Y ) * INV_PI) );
	}

	CRsElementDesc& CVertexShader::Process( const std::shared_ptr<CMesh> pMesh )
	{
		m_vsOutput.m_vertices.clear();
		m_vsOutput.m_vertices.reserve( pMesh->m_nVerties );
		m_vsOutput.m_texCoords.reserve( pMesh->m_nVerties );

		Matrix4 wvpMatrix = m_worldMatrix * m_viewMatrix * m_projectionMatrix;

		for ( int i = 0; i < pMesh->m_nVerties; ++i )
		{
			Vector4 position = pMesh->m_vertices[i];

			m_vsOutput.m_texCoords.emplace_back( CalcShpericalTexCoord( position ) ); 

			if ( pMesh->m_coordinate == COORDINATE::OBJECT_COORDINATE )
			{
				position.Transform( wvpMatrix );
			}

			m_vsOutput.m_vertices.emplace_back( position );
			
		}
		
		unsigned int key = 0;
		m_vsOutput.m_faces.clear();
		for ( std::vector<Face>::iterator faceiter = pMesh->m_faces.begin( ); faceiter != pMesh->m_faces.end( ); ++faceiter )
		{
			key = faceiter - pMesh->m_faces.begin( );
			if ( m_vsOutput.m_faces.size( ) <= key )
			{
				m_vsOutput.m_faces.emplace_back( );
				m_vsOutput.m_faces[key].reserve( faceiter->m_indices.size( ) );
			}
			for ( std::vector<int>::iterator indexiter = faceiter->m_indices.begin( ); indexiter != faceiter->m_indices.end( ); ++indexiter )
			{
				m_vsOutput.m_faces[key].emplace_back( *indexiter );
			}
		}

		m_vsOutput.m_coordinate = pMesh->m_coordinate;

		return m_vsOutput;
	}

	CRsElementDesc& CVertexShader::ProcessParallel( const std::shared_ptr<CMesh> pMesh, CXtzThreadPool* threadPool )
	{
		m_vsOutput.m_vertices.clear( );
		m_vsOutput.m_vertices.resize( pMesh->m_nVerties );
		m_vsOutput.m_texCoords.resize( pMesh->m_nVerties );

		m_wvpMatrix = m_worldMatrix * m_viewMatrix * m_projectionMatrix;

		for ( int i = 0; i < pMesh->m_nVerties; ++i )
		{
			VsThreadArg* pArg = new VsThreadArg;

			pArg->index = i;
			pArg->pVs = this;
			pArg->pMesh = pMesh;

			m_vsOutput.m_texCoords.emplace_back( 0.0f, 0.0f ); //임시 코드
			threadPool->AddWork( VsThreadWork, (LPVOID)pArg );
		}

		unsigned int key = 0;
		m_vsOutput.m_faces.clear();
		for ( std::vector<Face>::iterator faceiter = pMesh->m_faces.begin( ); faceiter != pMesh->m_faces.end( ); ++faceiter )
		{
			key = faceiter - pMesh->m_faces.begin( );
			if ( m_vsOutput.m_faces.size( ) <= key )
			{
				m_vsOutput.m_faces.emplace_back( );
				m_vsOutput.m_faces[key].reserve( faceiter->m_indices.size() );
			}
			for ( std::vector<int>::iterator indexiter = faceiter->m_indices.begin( ); indexiter != faceiter->m_indices.end( ); ++indexiter )
			{
				m_vsOutput.m_faces[key].emplace_back( *indexiter );
			}
		}

		m_vsOutput.m_coordinate = pMesh->m_coordinate;

		threadPool->WaitThread( );

		m_vsOutput.m_cameraPos.W = 1;
		m_vsOutput.m_cameraPos.X = -m_viewMatrix.A[4][1];
		m_vsOutput.m_cameraPos.Y = -m_viewMatrix.A[4][2];
		m_vsOutput.m_cameraPos.Z = -m_viewMatrix.A[4][3];

		return m_vsOutput;
	}

	void VsThreadWork( LPVOID arg )
	{
		VsThreadArg* pVsarg = (VsThreadArg*)arg;

		Vector4& vector = pVsarg->pVs->GetOutputVertex( pVsarg->index );
		vector = pVsarg->pMesh->m_vertices[pVsarg->index];
		
		Vector2& texCoord = pVsarg->pVs->GetOutputTexCoord( pVsarg->index );
		texCoord = CalcShpericalTexCoord( vector );

		if ( pVsarg->pMesh->m_coordinate == COORDINATE::OBJECT_COORDINATE )
		{
			vector.Transform( pVsarg->pVs->GetwvpMatrix( ) );
		}

		delete pVsarg;
	}
}
