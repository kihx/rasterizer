#include "CoolD_CustomMesh.h"
#include "..\Data\CoolD_Inlines.h"
#include "..\Data\CoolD_Defines.h"
#include "CoolD_Transform.h"
#include <fstream>
#include <sstream>

namespace CoolD
{
	CustomMesh::CustomMesh(const CustomMesh& rhs)
	{
		*this = rhs;
	}

	Vector3& CustomMesh::GetVertex(Duint index)
	{
		return m_vecVertex[ index - 1 ];	//정점 정보는 1부터 시작하지만 벡터는 0부터 시작하기 때문에
	}

	BaseFace& CustomMesh::GetFace(Duint index)
	{
		return m_vecFace[ index - 1 ];
	}

	Duint CustomMesh::GetVertexSize() const
	{
		return m_vecVertex.size();
	}

	Duint CustomMesh::GetFaceSize() const
	{
		return m_vecFace.size();
	}
	
	vector<Vector3>* CustomMesh::GetVectorVertex()
	{
		return &m_vecVertex;
	}

	vector<BaseFace>* CustomMesh::GetVectorFace()
	{
		return &m_vecFace;
	}		

	void CustomMesh::SetVectorVertex(vector<Vector3>& vecVertex)
	{
		m_vecVertex.swap( vecVertex );
	}

	void CustomMesh::SetVectorFace( vector<BaseFace>& vecFace )
	{
		m_vecFace.swap( vecFace );
	}		

	Dbool CustomMeshMSH::Load(const Dchar* filename)
	{		
		Dint vectexCount = 0;
		Dint faceCount = 0;

		fstream readStream(filename);
		istreambuf_iterator<char> begin(readStream);
		istreambuf_iterator<char> end;

		string strBuffer(vector<char>(begin, end).data());
		stringstream sstream(strBuffer);

		while( !sstream.eof() )
		{
			string strToken;
			sstream >> strToken;

			if( strToken == "#$Vertices" )
			{
				sstream >> vectexCount;
				m_vecVertex.reserve(vectexCount);				
			}
			else if( strToken == "#$Faces" )
			{
				sstream >> faceCount;
				m_vecFace.reserve(faceCount);
			}
			else if( strToken == "Vertex" )
			{
				Dint vertexNum;
				sstream >> vertexNum;

				assert(0 < vertexNum && vertexNum <= vectexCount);	//지정된 형식과 다를경우 kill

				Vector3 v;
				sstream >> v.x >> v.y >> v.z;
				v.x = ROUND_Off(v.x, 0);
				v.y = ROUND_Off(v.y, 0);
				v.z = ROUND_Off(v.z, 0);

				m_vecVertex.emplace_back(v);
			}
			else if( strToken == "Face" )
			{
				Dint faceNum;
				sstream >> faceNum;

				assert(0 < faceNum && faceNum <= faceCount);

				BaseFace f;
				Dint readR, readG, readB;
				sstream >> readR >> readG >> readB;
				f.color.r = readR & 0x000000ff;
				f.color.g = readG & 0x000000ff;
				f.color.b = readB & 0x000000ff;
				f.color.a = 0x000000ff;

				Dint indexCount = 0;
				sstream >> indexCount;

				for( Dint i = 0; i < indexCount; ++i )
				{
					Dint vertexNum;
					sstream >> vertexNum;

					assert(0 < vertexNum && vertexNum <= vectexCount);

					f.vecIndex.push_back(vertexNum);
				}

				m_vecFace.emplace_back(f);
			}
		}
		return true;
	}			

	MeshType CustomMeshMSH::GetType() const
	{
		return MSH;
	}

	CustomMesh* CustomMeshMSH::Clone()
	{
		return new CustomMeshMSH(*this);
	}

	CustomMeshMSH::CustomMeshMSH(const CustomMeshMSH& rhs) :
		CustomMesh( rhs )
	{
		
	}

	Dbool CustomMeshPLY::Load(const Dchar* filename)
	{		
		Dint vectexCount = 0;
		Dint faceCount = 0;

		fstream readStream(filename);
		istreambuf_iterator<char> begin(readStream);
		istreambuf_iterator<char> end;

		string strBuffer(vector<char>(begin, end).data());
		stringstream sstream(strBuffer);

		while( !sstream.eof() )
		{
			string strToken;
			sstream >> strToken;

			if( strToken == "#$Vertices" )
			{
				sstream >> vectexCount;
				m_vecVertex.reserve(vectexCount);				
			}
			else if( strToken == "#$Faces" )
			{
				sstream >> faceCount;
				m_vecFace.reserve(faceCount);
			}
			else if( strToken == "Vertex" )
			{
				Dint vertexNum;
				sstream >> vertexNum;
				
				assert(0 < vertexNum && vertexNum <= vectexCount);	//지정된 형식과 다를경우 kill

				Vector3 v;
				sstream >> v.x >> v.y >> v.z;			

				m_vecVertex.emplace_back(v);
			}
			else if( strToken == "Face" )
			{
				Dint faceNum;
				sstream >> faceNum;

				assert(0 < faceNum && faceNum <= faceCount);
				RandomGenerator<int> rand(0, 255);
				
				BaseFace f;
				f.color = { (Duchar)rand.GetRand(), (Duchar)rand.GetRand(), (Duchar)rand.GetRand(), 255 };
				
				for( Dint i = 0; i < 3; ++i )
				{
					Dint vertexNum;
					sstream >> vertexNum;

					assert(0 < vertexNum && vertexNum <= vectexCount);
					f.vecIndex.push_back(vertexNum);
				}

				m_vecFace.emplace_back(f);
			}
		}
		return true;
	}		

	MeshType CustomMeshPLY::GetType() const
	{
		return PLY;
	}

	CustomMeshPLY::CustomMeshPLY(const CustomMeshPLY& rhs):
		CustomMesh(rhs)
	{

	}

	CustomMesh* CustomMeshPLY::Clone()
	{
		return new CustomMeshPLY(*this);
	}	
}
