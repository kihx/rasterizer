#include "stdafx.h"
#include "VertexShader.h"

namespace xtozero
{
	CRsElementDesc& CVertexShader::Process( const std::shared_ptr<CMesh> pMesh )
	{
		m_vsOutput.m_vertices.clear();
		m_vsOutput.m_vertices.reserve( pMesh->m_nVerties );

		Matrix4& wvpMatrix = m_worldMatrix * m_viewMatrix * m_projectionMatrix;

		for ( int i = 0; i < pMesh->m_nVerties; ++i )
		{
			Vector3 position = pMesh->m_vertices[i];
			if ( pMesh->m_coordinate == COORDINATE::OBJECT_COORDINATE )
			{
				position.Transform( wvpMatrix );
			}

			m_vsOutput.m_vertices.emplace_back( position.X, position.Y, position.Z );
		}
		
		int key = 0;
		m_vsOutput.m_faces.clear();
		for ( std::vector<Face>::iterator& faceiter = pMesh->m_faces.begin( ); faceiter != pMesh->m_faces.end( ); ++faceiter )
		{
			key = faceiter - pMesh->m_faces.begin( );
			if ( m_vsOutput.m_faces.size( ) <= key )
			{
				m_vsOutput.m_faces.emplace_back( );
				m_vsOutput.m_faces[key].reserve( faceiter->m_indices.size( ) );
			}
			for ( std::vector<int>::iterator& indexiter = faceiter->m_indices.begin( ); indexiter != faceiter->m_indices.end( ); ++indexiter )
			{
				m_vsOutput.m_faces[key].emplace_back( *indexiter );
			}
		}

		m_vsOutput.m_coodinate = pMesh->m_coordinate;

		return m_vsOutput;
	}

	CRsElementDesc& CVertexShader::ProcessParallel( const std::shared_ptr<CMesh> pMesh, std::shared_ptr<xtozero::CXtzThreadPool> threadPool )
	{
		m_vsOutput.m_vertices.clear( );
		m_vsOutput.m_vertices.resize( pMesh->m_nVerties );

		Matrix4& wvpMatrix = m_worldMatrix * m_viewMatrix * m_projectionMatrix;

		for ( int i = 0; i < pMesh->m_nVerties; ++i )
		{
			VsThreadArg* pArg = new VsThreadArg;

			pArg->index = i;
			pArg->pVs = this;
			pArg->matrix = wvpMatrix;
			pArg->pMesh = pMesh;

			threadPool->AddWork( VsThreadWork, (LPVOID)pArg );
		}
		threadPool->Run();

		int key = 0;
		m_vsOutput.m_faces.clear();
		for ( std::vector<Face>::iterator& faceiter = pMesh->m_faces.begin( ); faceiter != pMesh->m_faces.end( ); ++faceiter )
		{
			key = faceiter - pMesh->m_faces.begin( );
			if ( m_vsOutput.m_faces.size( ) <= key )
			{
				m_vsOutput.m_faces.emplace_back( );
				m_vsOutput.m_faces[key].reserve( faceiter->m_indices.size() );
			}
			for ( std::vector<int>::iterator& indexiter = faceiter->m_indices.begin( ); indexiter != faceiter->m_indices.end( ); ++indexiter )
			{
				m_vsOutput.m_faces[key].emplace_back( *indexiter );
			}
		}

		m_vsOutput.m_coodinate = pMesh->m_coordinate;

		return m_vsOutput;
	}

	void CVertexShader::InsertTransformedVertex( Vector3& pos, int index )
	{
		m_vsOutput.m_vertices[index].X = pos.X;
		m_vsOutput.m_vertices[index].Y = pos.Y;
		m_vsOutput.m_vertices[index].Z = pos.Z;
	}
}
