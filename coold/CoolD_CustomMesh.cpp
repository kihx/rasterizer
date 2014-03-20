#include "CoolD_CustomMesh.h"
#include "CoolD_Inlines.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>

namespace CoolD
{
	CustomMesh::CustomMesh( const Dchar* filename ) 
		: m_szFileName(filename)
	{
	}
	CustomMesh::~CustomMesh()
	{		
	}

	CustomMesh* CustomMesh::CreateMeshFromFile( const Dchar* filename )
	{
		CustomMesh* pMesh = new CustomMesh( filename );
		if( pMesh && pMesh->LoadMeshInfo() )
		{
			return pMesh;
		}

		Safe_Delete(pMesh);
		return nullptr;
	}	

	bool CustomMesh::LoadMeshInfo()
	{
		if( m_szFileName == nullptr )
		{
			return false;
		}

		Dint vectexCount = 0;
		Dint faceCount	= 0;

		fstream readStream( m_szFileName );
		istreambuf_iterator<char> begin( readStream );
		istreambuf_iterator<char> end;
		
		string strBuffer( vector<char>(begin, end).data() );
		stringstream sstream(strBuffer);
		
		while( !sstream.eof() )
		{
			string strToken;
			sstream >> strToken;

			if( strToken == "#$Vertices" )
			{
				sstream >> vectexCount;
				m_vecVertex.reserve( vectexCount );
			}
			else if( strToken == "#$Faces") 
			{
				sstream >> faceCount;	
				m_vecFace.reserve( faceCount );
			}
			else if( strToken == "Vertex" )
			{
				Dint vertexNum;
				sstream >> vertexNum;

				assert( 0 < vertexNum && vertexNum <= vectexCount );	//지정된 형식과 다를경우 kill

				baseVertex v; 
				sstream >> v.x >> v.y >> v.z;
				m_vecVertex.push_back(v);				
			}
			else if( strToken == "Face" )
			{
				Dint faceNum;				
				sstream >> faceNum;

				assert( 0 < faceNum && faceNum <= faceCount );

				baseFace f;
				Dint readR, readG, readB;
				sstream >> readR >> readG >> readB;
				f.r = readR & 0x000000ff;
				f.g = readG & 0x000000ff;
				f.b = readB & 0x000000ff;
				f.a = 0x000000ff;

				Dint indexCount = 0;
				sstream>>indexCount;

				for( Dint i = 0; i < indexCount; ++i)
				{
					Dint vertexNum;
					sstream>>vertexNum;

					assert( 0 < vertexNum && vertexNum <= vectexCount );

					f.vecIndex.push_back( vertexNum );
				}

				m_vecFace.push_back(f);
			}		
		}
		return true;
	}

	const baseVertex& CustomMesh::GetVertex( Duint index ) const
	{
		return m_vecVertex[index - 1];	//정점 정보는 1부터 시작하지만 벡터는 0부터 시작하기 때문에
	}

	const baseFace& CustomMesh::GetFace( Duint index ) const
	{
		return m_vecFace[index - 1];
	}

	Duint CustomMesh::GetVertexSize() const
	{
		return m_vecVertex.size();
	}

	Duint CustomMesh::GetFaceSize() const
	{
		return m_vecFace.size();
	}
}
