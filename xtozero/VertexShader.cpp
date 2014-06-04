#include "stdafx.h"
#include "VertexShader.h"

#include <cmath>

namespace xtozero
{
	CRsElementDesc& CVertexShader::Process( const std::shared_ptr<CMesh> pMesh )
	{
		m_vsOutput.m_vertices.clear();
		m_vsOutput.m_vertices.reserve( pMesh->m_nVerties );
		m_vsOutput.m_localVertices.reserve( pMesh->m_nVerties );

		Matrix4 wvpMatrix = m_worldMatrix * m_viewMatrix * m_projectionMatrix;

		for ( int i = 0; i < pMesh->m_nVerties; ++i )
		{
			Vector4 position = pMesh->m_vertices[i];

			m_vsOutput.m_localVertices.emplace_back( position.X, position.Y, position.Z );

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
		m_vsOutput.m_localVertices.resize( pMesh->m_nVerties );

		m_wvpMatrix = m_worldMatrix * m_viewMatrix * m_projectionMatrix;

		for ( int i = 0; i < pMesh->m_nVerties; ++i )
		{
			VsThreadArg* pArg = new VsThreadArg;

			pArg->index = i;
			pArg->pVs = this;
			pArg->pMesh = pMesh;

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
		
		Vector3& texCoord = pVsarg->pVs->GetOutputLocalVertex( pVsarg->index );
		texCoord.X = vector.X;
		texCoord.Y = vector.Y;
		texCoord.Z = vector.Z;

		if ( pVsarg->pMesh->m_coordinate == COORDINATE::OBJECT_COORDINATE )
		{
			vector.Transform( pVsarg->pVs->GetwvpMatrix( ) );
		}

		delete pVsarg;
	}
}
