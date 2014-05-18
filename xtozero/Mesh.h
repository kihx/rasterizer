#ifndef _MESH_H_
#define _MESH_H_

#include "FileHandler.h"
#include "XtzMath.h"

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <iostream>

const int VERTEX_ELEMENT_COUNT = 3;
const int COLOR_ELEMENT_COUNT = 3;

namespace xtozero
{
	//enum VERTEX_ELEMENT
	//{
	//	x = 0,
	//	y,
	//	z
	//};

	//template<typename T>
	//struct Vertex
	//{
	//	T m_element[VERTEX_ELEMENT_COUNT];
	//};

	enum class COORDINATE
	{
		OBJECT_COORDINATE = 0,
		WINDOW_COORDINATE
	};

	enum COLOR_ELEMENT
	{
		r = 0,
		g,
		b
	};

	struct Face
	{
		unsigned char m_color[COLOR_ELEMENT_COUNT];
		std::vector<int> m_indices;
	};

	//typedef Vertex<float> VERTEXF;

	class CMesh
	{
	public:
		std::vector<Vector4> m_vertices;
		std::vector<Vector2> m_texCoords;
		std::vector<Face> m_faces;

		int m_nfaces;
		int m_nVerties;

		COORDINATE m_coordinate;

		CMesh( void ) : m_nfaces( 0 ), m_nVerties( 0 ) {}
		~CMesh( void ){}

		bool LoadFromFile( const char* pfilename );
		bool LoadFromMsh( const char* pfilename );
		bool LoadFromPly( const char* pfilename );
		void PrintMeshInfo( void );
	};

	class CMeshManager
	{
	private:
		std::string m_recentMesh;
		std::map<std::string, std::shared_ptr<CMesh>> m_meshes;
	public:
		CMeshManager( void ){}
		~CMeshManager( void ){}

		std::shared_ptr<CMesh> LoadMeshFromFile( const char* pfilename );
		std::shared_ptr<CMesh> LoadRecentMesh();
	};
}

#endif
