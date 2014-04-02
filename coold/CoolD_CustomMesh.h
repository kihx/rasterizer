#pragma once

#include "CoolD_Type.h"
#include "CoolD_Struct.h"

namespace CoolD
{
	class CustomMesh
	{	
	public:
		CustomMesh() = default;
		virtual ~CustomMesh() = default;
		CustomMesh(const CustomMesh& rhs);

	public:
		const BaseVertex& GetVertex(Duint index) const;
		const BaseFace&	GetFace(Duint index) const;
		Duint GetVertexSize() const;
		Duint GetFaceSize() const;		
		const vector<BaseVertex>& GetVectorVertex() const;
		const vector<BaseFace>& GetVectorFace() const;
		Dvoid SetVectorVertex(vector<BaseVertex>& vecVertex );
		Dvoid SetVectorFace(vector<BaseFace>& vecFace );

	public:
		virtual CustomMesh* Clone() = 0;
		virtual Dbool Load(const Dchar* filename) = 0;			
		virtual CustomMesh* GetTransformMesh() = 0;

	protected:
		vector<BaseVertex> m_vecVertex;
		vector<BaseFace>	m_vecFace;
	};

	class CustomMeshMSH : public CustomMesh
	{
	public:
		virtual CustomMesh* Clone();		
		virtual Dbool Load(const Dchar* filename);
		virtual CustomMesh* GetTransformMesh();
	};

	class CustomMeshPLY : public CustomMesh
	{
	public:
		virtual CustomMesh* Clone();		
		virtual Dbool Load(const Dchar* filename);
		virtual CustomMesh* GetTransformMesh();
	};
};