#ifndef _MESH_H_
#define _MESH_H_

#include <vector>
#include <map>
#include <string>
#include <memory>

#define VETEX_ELEMENT_COUNT 3
#define COLOR_ELEMENT_COUNT 3

namespace xtozero
{
	enum VERTEX_ELMENT
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

	enum COLOR_ELMENT
	{
		r = 0,
		g,
		b
	};

	struct Face
	{
		unsigned char color[COLOR_ELEMENT_COUNT];
		std::vector<int> indices;
	};

	typedef Vertex<float> VERTEXF;

	class CMesh
	{
	private:
		std::vector<VERTEXF> m_vertices;
		std::vector<Face> m_faces;
	public:
		CMesh( void ){}
		~CMesh( void ){}
	};

	class CMeshManager
	{
	private:
		static CMeshManager* m_instance;
		std::map<std::string, std::shared_ptr<CMesh>> m_meshes;
	public:
		CMeshManager( void ){}
		~CMeshManager( void ){}

		static CMeshManager* GetInstance( void );
		static void ReleaseInstance( void );
		
		std::shared_ptr<CMesh> LoadMeshFromFile( const char* pfilename );
	};
}

#endif
