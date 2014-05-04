#include "CoolD_Transform.h"
#include "..\Data\CoolD_Inlines.h"
#include "..\Math\CoolD_Matrix33.h"
#include "..\Data\CoolD_Defines.h"
#include "..\Data\CoolD_Struct.h"

namespace CoolD
{
	//perspective -> viewport
	Matrix44 TransformHelper::CreateViewport(Dint left, Dint bottom, Dint width, Dint height)
	{
		Matrix44 matViewPort;

		matViewPort(0, 0) = ((Dfloat)width / 2);
		matViewPort(0, 3) = ((Dfloat)left + width / 2);
		matViewPort(1, 1) = ((Dfloat)-height / 2);
		matViewPort(1, 3) = ((Dfloat)bottom + height / 2);
		matViewPort(2, 2) = ((Dfloat)1 / 2);
		matViewPort(2, 3) = ((Dfloat)1 / 2);
		matViewPort(3, 3) = ((Dfloat)1);

		return matViewPort;
	}	

	//world -> view
	Matrix44 TransformHelper::CreateView(const Vector3& eye, const Vector3& lookAt, const Vector3& up)
	{				
		// compute view vectors
		Vector3 view = lookAt - eye;
		Vector3 right;
		Vector3 viewUp;
		view.Normalize();
		right = view.Cross(up);
		right.Normalize();
		viewUp = right.Cross(view);
		viewUp.Normalize();

		// now set up matrices
		// base rotation matrix
		Matrix33 rotate;
		rotate.SetColumns(right, viewUp, -view);

		// view->world transform
		// set rotation
		//Matrix44 mViewToWorldMatrix;
		//mViewToWorldMatrix.Rotation(rotate);
		//// set translation (eye position)
		//mViewToWorldMatrix(0, 3) = eye.x;
		//mViewToWorldMatrix(1, 3) = eye.y;
		//mViewToWorldMatrix(2, 3) = eye.z;

		// world->view transform
		// set rotation
		Matrix44 mWorldToViewMatrix;
		rotate.Transpose();
		mWorldToViewMatrix.Rotation(rotate);
		// set translation (rotate into view space)
		Vector3 invEye = -(rotate*eye);
		mWorldToViewMatrix(0, 3) = invEye.x;
		mWorldToViewMatrix(1, 3) = invEye.y;
		mWorldToViewMatrix(2, 3) = invEye.z;
					
		return mWorldToViewMatrix;
	}

	//world -> perspective
	Matrix44 TransformHelper::CreatePerspective(Dfloat fov, Dfloat aspect, Dfloat nearZ, Dfloat farZ)
	{
		Dfloat d = 1.0f / Tan(fov * 0.5f);
		Dfloat recip = 1.0f / (nearZ - farZ);
		Matrix44 matPerspective;

		matPerspective(0, 0) = d / aspect;
		matPerspective(1, 1) = d;
		matPerspective(2, 2) = (nearZ + farZ)*recip;
		matPerspective(2, 3) = 2 * nearZ*farZ*recip;
		matPerspective(3, 2) = -1.0f;
		matPerspective(3, 3) = 0.0f;

		return matPerspective;
	}

	//local->world
	Matrix44 TransformHelper::CreateWorld(initializer_list<Matrix44> matrixOrder)
	{	//스케일, 자전, 이동, 공전, 부모 순으로 넣으면 된다.
		Matrix44 matWorld;
		for( auto iter = matrixOrder.begin(); iter != matrixOrder.end(); ++iter )
		{
			matWorld = (*iter) * matWorld;
		}

		return matWorld;
	}
	
	Vector3 TransformHelper::TransformWVP(const array<Matrix44, TRANSFORM_END>& arrayTransform, Vector3 vertex)
	{			
		Vector4 v = arrayTransform[ PERSPECTIVE ] * arrayTransform[ VIEW ] * arrayTransform[ WORLD ] * Vector4(vertex, 1);		
		return Vec4ToVec3(v, Vector4::W_DIVIDE);
	}

	Matrix33 TransformHelper::CreatePerspectNDCtoView(Dfloat fov, Dfloat aspect, Dfloat sx, Dfloat sy, Dfloat width, Dfloat height )
	{
		Dfloat d = 1.0f / Tan(fov * 0.5f);		
		Matrix33 matPerspectNDCtoView;

		matPerspectNDCtoView(0, 0) = 2.0f * aspect / width;
		matPerspectNDCtoView(0, 2) = (-2.0f * aspect / width) * sx - 1;
		matPerspectNDCtoView(1, 1) = -2.0f / height;
		matPerspectNDCtoView(1, 2) = (2.0f / height) * sy + 1;
		matPerspectNDCtoView(2, 2) = -d;

		return matPerspectNDCtoView;
	}

	Vector3 TransformHelper::TransformViewport(const array<Matrix44, TRANSFORM_END>& arrayTransform, Vector3& vertex)
	{		
		Vector4 v = arrayTransform[ VIEWPORT ] * Vector4(vertex, 1);
		return Vec4ToVec3(v, Vector4::W_IGNORE);
	}
};