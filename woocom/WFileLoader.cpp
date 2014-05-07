#include "WFileLoader.h"

#include "WTrDatai.h"
#include "WPolyData.h"
#include "Math.h"

#include <fstream>

WTriData* FileLoader::LoadMesh(const char* fileName)
{
	std::fstream stream(fileName);

	const int MAX_LEN = 1024;
	char buffer[MAX_LEN] = { 0 };

	int vertexNum = 0;
	int faceNum = 0;

	WTriData* triangle = new WTriData();

	while (stream.good())
	{
		stream >> buffer;

		if (strstr(buffer, "#$"))
		{
			if (strncmp(&buffer[2], "Vertices", 8) == 0)
			{
				stream >> vertexNum;
				triangle->SetVertexNum(vertexNum);
			}
			else if (strncmp(&buffer[2], "Faces", 5) == 0)
			{
				stream >> faceNum;
				triangle->SetFaceNum(faceNum);
			}
		}
		else if (strncmp(buffer, "Vertex", 6) == 0)
		{
			int vertexID = 0;
			VERTEX* vertex = new VERTEX();
			stream >> vertexID >> vertex->m_pos[0] >> vertex->m_pos[1] >> vertex->m_pos[2];
			triangle->PushVertex(vertex);
		}
		else if (strncmp(buffer, "Face", 4) == 0)
		{
			int faceID = 0;
			int indexNum = 0;
			int r, g, b;
			WFace* face = new WFace();
			stream >> faceID >> r >> g >> b;
			face->m_rgb[0] = (char)r;
			face->m_rgb[1] = (char)g;
			face->m_rgb[2] = (char)b;

			stream >> indexNum;
			for (int i = 0; i< indexNum; ++i)
			{
				int vertexID = 0;
				stream >> vertexID;
				face->m_index.push_back(vertexID - 1);
			}
			triangle->PushFace(face);
		}
	}

	return triangle;
}

WPolyData* FileLoader::LoadPoly(const char* fileName)
{
	std::fstream stream(fileName);

	const int MAX_LEN = 1024;
	char buffer[MAX_LEN] = { 0 };

	int vertexNum = 0;
	int faceNum = 0;

	WPolyData* poly = new WPolyData();

	while (stream.good())
	{
		stream >> buffer;

		if (strstr(buffer, "#$"))
		{
			if (strncmp(&buffer[2], "Vertices", 8) == 0)
			{
				stream >> vertexNum;
				poly->SetVertexNum(vertexNum);
			}
			else if (strncmp(&buffer[2], "Faces", 5) == 0)
			{
				stream >> faceNum;
				poly->SetFaceNum(faceNum);
			}
		}
		else if (strncmp(buffer, "Vertex", 6) == 0)
		{
			int vertexID = 0;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			stream >> vertexID >> x >> y >> z;

			Vector3* vertex = new Vector3(x,y,z);
			poly->PushVertex(vertex);
		}
		else if (strncmp(buffer, "Face", 4) == 0)
		{
			int faceID = 0;
			stream >> faceID;

			WPolyFace* face = new WPolyFace();
			for (int i = 0; i< 3; ++i)
			{
				int vertexID = 0;
				stream >> vertexID;
				face->m_indices.push_back(vertexID - 1);
				poly->PushIndex(vertexID - 1);
			}
			poly->PushFace(face);
		}
	}
	return poly;
}