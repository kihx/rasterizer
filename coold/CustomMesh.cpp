#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include "CustomMesh.h"

using namespace std;

namespace coold
{
	CustomMesh* CustomMesh::CreateMeshFromFile( const char* filename )
	{
		CustomMesh* pMesh = new CustomMesh( filename );
		if( pMesh && pMesh->LoadMeshInfo() )
		{
			return pMesh;
		}
		return nullptr;
	}
		
	CustomMesh::CustomMesh( const char* filename ) 
	: m_szFileName(filename)
	{
	}

	bool CustomMesh::LoadMeshInfo()
	{
		if( m_szFileName == nullptr )
		{
			return false;
		}

		int vectexCount = 0;
		int faceCount	= 0;

		fstream readStream( m_szFileName );
		istreambuf_iterator<char> begin( readStream );
		istreambuf_iterator<char> end;
		
		string strBuffer( std::vector<char>(begin, end).data() );
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
				int vertexNum;
				sstream >> vertexNum;

				assert( 0 < vertexNum && vertexNum <= vectexCount );	//지정된 형식과 다를경우 kill

				baseVertex v; 
				sstream >> v.x >> v.y >> v.z;
				m_vecVertex.push_back(v);				
			}
			else if( strToken == "Face" )
			{
				int faceNum;				
				sstream >> faceNum;

				assert( 0 < faceNum && faceNum <= faceCount );

				baseFace f;
				int readR, readG, readB;
				sstream >> readR >> readG >> readB;
				f.r = readR & 0x000000ff;
				f.g = readG & 0x000000ff;
				f.b = readB & 0x000000ff;
				f.a = 0x000000ff;

				int indexCount = 0;
				sstream>>indexCount;

				for( int i = 0; i < indexCount; ++i)
				{
					int vertexNum;
					sstream>>vertexNum;

					assert( 0 < vertexNum && vertexNum <= vectexCount );

					f.vecIndex.push_back( vertexNum );
				}
			}		
		}
		return true;
	}

}
