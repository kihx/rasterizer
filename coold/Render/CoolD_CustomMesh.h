#pragma once

#include "..\Data\CoolD_Type.h"
#include "..\Data\CoolD_Struct.h"
#include "..\Math\CoolD_Matrix44.h"

namespace CoolD
{
	class CustomMesh
	{	
	public:
		CustomMesh() = default;
		virtual ~CustomMesh() = default;
		CustomMesh(const CustomMesh& rhs);

	public:
		Vector3& GetVertex(Duint index);
		BaseFace&	GetFace(Duint index);
		Duint GetVertexSize() const;
		Duint GetFaceSize() const;		
		vector<Vector3>* GetVectorVertex() ;
		vector<BaseFace>* GetVectorFace();
		Dvoid SetVectorVertex(vector<Vector3>& vecVertex);
		Dvoid SetVectorFace(vector<BaseFace>& vecFace );				

	public:				
		virtual Dbool Load(const Dchar* filename) = 0;			
		virtual MeshType GetType() const = 0;
		virtual CustomMesh* Clone() = 0;
		
	protected:
		vector<Vector3>		m_vecVertex;
		vector<BaseFace>	m_vecFace;		
	};

	class CustomMeshMSH : public CustomMesh
	{
	public:
		CustomMeshMSH() = default;
		CustomMeshMSH(const CustomMeshMSH& rhs);
		virtual CustomMesh* Clone();
		virtual Dbool Load(const Dchar* filename);		
		virtual MeshType GetType() const;
	};

	class CustomMeshPLY : public CustomMesh
	{
	public:		
		CustomMeshPLY() = default;
		CustomMeshPLY(const CustomMeshPLY& rhs);
		virtual CustomMesh* Clone();
		virtual Dbool Load(const Dchar* filename);		
		virtual MeshType GetType() const;
	};
};