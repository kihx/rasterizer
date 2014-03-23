#ifndef _MESH_H_
#define _MESH_H_

#include "FileHandler.h"

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <iostream>

const int VETEX_ELEMENT_COUNT = 3;
const int COLOR_ELEMENT_COUNT = 3;

namespace xtozero
{
	enum VERTEX_ELEMENT
	{
		x = 0,
		y,
		z
	};

	template<typename T>
	struct Vertex
	{
		T m_element[VETEX_ELEMENT_COUNT];
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

	typedef Vertex<float> VERTEXF;

	class CMesh
	{
	public:
		std::vector<VERTEXF> m_vertices;
		std::vector<Face> m_faces;

		CMesh( void ){}
		~CMesh( void ){}

		bool LoadFromFile( const char* pfilename );
		void PrintMeshInfo( void );
	};

	class CMeshManager
	{
	private:
		std::string m_recentMesh;
		static CMeshManager* m_instance;
		std::map<std::string, std::shared_ptr<CMesh>> m_meshes;
	public:
		CMeshManager( void ){}
		~CMeshManager( void ){}

		static CMeshManager* GetInstance( void );
		static void ReleaseInstance( void );

		std::shared_ptr<CMesh> LoadMeshFromFile( const char* pfilename );
		std::shared_ptr<CMesh> LoadRecentMesh();
	};
}

#endif
