#pragma once

#include "..\Data\CoolD_Type.h"
#include "..\Math\CoolD_Matrix44.h"
#include "..\Data\CoolD_Singleton.h"
#include "..\Data\CoolD_Struct.h"

namespace CoolD
{
	class TransformHelper final
	{
	public:
		static Matrix44 CreateWorld( initializer_list<Matrix44> matrixOrder );
		static Matrix44 CreateView(const Vector3& eye, const Vector3& lookAt, const Vector3& up);
		static Matrix44 CreatePerspective(Dfloat fov, Dfloat aspect, Dfloat nearZ, Dfloat farZ);
		static Matrix44 CreateViewport(Dint left, Dint bottom, Dint width, Dint height);		
		static Matrix33 CreatePerspectNDCtoView(Dfloat fov, Dfloat aspect, Dfloat sx, Dfloat sy, Dfloat width, Dfloat height);
		static Vector3	TransformWVP(const array<Matrix44, TRANSFORM_END>& arrayTransform, Vector3 vertex);	
		static Vector3	TransformViewport(const array<Matrix44, TRANSFORM_END>& arrayTransform, Vector3& vertex);
	};
};