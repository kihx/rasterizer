#include "CoolD_Transform.h"
#include "CoolD_Inlines.h"
#include "CoolD_Matrix33.h"
#include "CoolD_Defines.h"

namespace CoolD
{
	Dvoid TransformHandler::SetTransform(TransType type, Matrix44& matTransform)
	{
		if( m_ArrayTransform[ type ] != nullptr )
		{
			Safe_Delete(m_ArrayTransform[ type ]);
			
		}

		m_ArrayTransform[ type ] = new Matrix44(matTransform);
	}

	Dvoid TransformHandler::CreateViewport(Dint left, Dint bottom, Dint width, Dint height)
	{
		if( m_ArrayTransform[ VIEWPORT ] )
		{
			return;
		}

		Matrix44 matViewPort;
		matViewPort(0, 0) = (Dfloat)(width / 2);
		matViewPort(0, 3) = (Dfloat)(left + width / 2);
		matViewPort(1, 1) = (Dfloat)(-height / 2);
		matViewPort(1, 3) = (Dfloat)(bottom + height / 2);
		matViewPort(2, 2) = (Dfloat)(1 / 2);
		matViewPort(2, 3) = (Dfloat)(1 / 2);
		matViewPort(3, 3) = (Dfloat)(1);

		SetTransform(VIEWPORT, matViewPort);
	}

	Matrix44 TransformHandler::GetTransform(TransType type)
	{
		assert(m_ArrayTransform[ type ]);
		return (*m_ArrayTransform[ type ]);
	}

	TransformHandler::TransformHandler()
	{
		for( int i = 0; i < TransType::END; ++i )
		{
			m_ArrayTransform[ i ] = nullptr;
		}		
	}

	TransformHandler::~TransformHandler()
	{
		for( int i = 0; i < TransType::END; ++i )
		{
			Safe_Delete(m_ArrayTransform[ i ]);
		}		
	}

	Dvoid TransformHandler::CreateView(const Vector3& eye, const Vector3& lookAt, const Vector3& up)
	{
		// compute view vectors
		Vector3 view = lookAt - eye;
		view.Normalize();

		Vector3 right = view.Cross(up);
		right.Normalize();

		Vector3 viewUp = right.Cross(view);
		viewUp.Normalize();

		// now set up matrices
		// world->view rotation
		Matrix33 rotate;
		rotate.SetRows(right, viewUp, -view);

		// world->view translation
		Vector3 xlate = -(rotate*eye);

		// build 4x4 matrix
		Matrix44 matView(rotate);
		matView(0, 3) = xlate.x;
		matView(1, 3) = xlate.y;
		matView(2, 3) = xlate.z;
		
		SetTransform(VIEW, matView);
	}

	Dvoid TransformHandler::CreatePerspective(float fov, float aspect, float nearZ, float farZ)
	{
		float d = 1.0f / Tan(fov * 0.5f);
		float recip = 1.0f / (nearZ - farZ);
		Matrix44 matPerspective;

		matPerspective(0, 0) = d / aspect;
		matPerspective(1, 1) = d;
		matPerspective(2, 2) = (nearZ + farZ)*recip;
		matPerspective(2, 3) = 2 * nearZ*farZ*recip;
		matPerspective(3, 2) = -1.0f;
		matPerspective(3, 3) = 0.0f;

		SetTransform(PERSPECTIVE, matPerspective);
	}

	Vector3 TransformHandler::TransformVertex(Vector3& vertex)
	{
		Vector4 transV4 = (*m_ArrayTransform[ VIEWPORT ]) * (*m_ArrayTransform[ PERSPECTIVE ]) * (*m_ArrayTransform[ VIEW ]) * (*m_ArrayTransform[ WORLD ]) * Vector4(vertex, 1);	

		FixLater(Perspective와 viewport 중간 과정은 다음에...)

		Vector3 transV3 = transV4.Vec4ToVec3( Vector4::W_DIVIDE );

		return transV3.Ceil();	//올림
	}
};