#pragma once
#include "..\Data\CoolD_Type.h"
#include "..\Data\CoolD_Struct.h"
#include "..\Math\CoolD_Matrix44.h"

namespace CoolD
{
	class FrustumCull
	{
	public:
		FrustumCull();
		~FrustumCull();

	public:		
		Dvoid CreateFrustum();
		Dbool CheckFrustumCull(const Vector3& vPoint);

	private:
		Vector4 CreatePlane(Vector4 v0, Vector4 v1, Vector4 v2);

	private:
		array<Vector4, 8> m_PlanePoints;
		array<Vector4, 6> m_Plane;		
	};
};