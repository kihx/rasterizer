#pragma once

#include "CoolD_Type.h"
#include "CoolD_Matrix44.h"
#include "CoolD_Singleton.h"


namespace CoolD
{
	class TransformHandler final : public CSingleton<TransformHandler>
	{
		friend class CSingleton<TransformHandler>;

	private:
		TransformHandler();

	public:
		~TransformHandler();

	public:
		Dvoid SetTransform(TransType type, Matrix44& matTransform);
		Matrix44 GetTransform(TransType type);

		Dvoid CreateView(const Vector3& eye, const Vector3& lookAt, const Vector3& up);
		Dvoid CreatePerspective(float fov, float aspect, float nearZ, float farZ);
		Dvoid CreateViewport(Dint left, Dint bottom, Dint width, Dint height);
		Vector3 TransformVertex(Vector3& vertex);

	private:
		array< Matrix44*, 4 > m_ArrayTransform;
	};
};