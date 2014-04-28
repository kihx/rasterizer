#include "CoolD_FrustumCull.h"

namespace CoolD
{
	FrustumCull::FrustumCull()		
	{
		/*
		4__________5
		│\		   │\
		│ \        │ \
		|6_\0______|7 \ 1
		\  │       \  │
		 \ │        \ │
		  \|2        \| 3
		   ────────────
		*/

		//근평면
		m_PlanePoints[ 0 ] = { -1,  1, -1, 1 };
		m_PlanePoints[ 1 ] = {  1,  1, -1, 1 };
		m_PlanePoints[ 2 ] = { -1, -1, -1, 1 };
		m_PlanePoints[ 3 ] = {  1, -1, -1, 1 };

		//원평면
		m_PlanePoints[ 4 ] = { -1,  1,  1, 1 };
		m_PlanePoints[ 5 ] = {  1,  1,  1, 1 };
		m_PlanePoints[ 6 ] = { -1, -1,  1, 1 };
		m_PlanePoints[ 7 ] = {  1, -1,  1, 1 };		
	}

	FrustumCull::~FrustumCull()
	{

	}

	void FrustumCull::CreateFrustum(const Matrix44& matView, const Matrix44& matPerspective)
	{
		Matrix44 invMat = (matPerspective * matView).AffineInverse();

		for( Duint i = 0; i < m_PlanePoints.size(); ++i )
		{	//투영 -> 뷰 -> 월드
			m_PlanePoints[ i ] = invMat * m_PlanePoints[ i ];
		}

		// 평면 만들기
		m_Plane[0] = CreatePlane(m_PlanePoints[ 0 ], m_PlanePoints[ 1 ], m_PlanePoints[ 2 ]);	//근평면
		m_Plane[1] = CreatePlane(m_PlanePoints[ 4 ], m_PlanePoints[ 5 ], m_PlanePoints[ 0 ]);	//상평면
		m_Plane[2] = CreatePlane(m_PlanePoints[ 2 ], m_PlanePoints[ 3 ], m_PlanePoints[ 6 ]);	//하평면
		m_Plane[3] = CreatePlane(m_PlanePoints[ 4 ], m_PlanePoints[ 0 ], m_PlanePoints[ 6 ]);	//좌평면
		m_Plane[4] = CreatePlane(m_PlanePoints[ 1 ], m_PlanePoints[ 5 ], m_PlanePoints[ 3 ]);	//우평면
		m_Plane[5] = CreatePlane(m_PlanePoints[ 5 ], m_PlanePoints[ 4 ], m_PlanePoints[ 7 ]);	//원평면	
	}

	Vector4 FrustumCull::CreatePlane(Vector4 v0, Vector4 v1, Vector4 v2)
	{
		Vector4 Plane;
		Vector3 rv1 = Vec4ToVec3(v1 - v0, Vector4::W_IGNORE);
		Vector3 rv2 = Vec4ToVec3(v2 - v0, Vector4::W_IGNORE);
		Vector3 rnv = rv1.Cross(rv2);
		rnv.Normalize();

		Plane.x = rnv.x;
		Plane.y = rnv.y;
		Plane.z = rnv.z;
		Plane.w = -rnv.Dot(Vec4ToVec3(v0, Vector4::W_IGNORE));
		return Plane;
	}

	Dbool FrustumCull::CheckFrustumCull(const Vector3& vPoint)
	{
		for( int i = 0; i < 6; ++i )
		{	//ax + by + cz +d = 0
			Dfloat fvalue = m_Plane[ i ].x * vPoint.x + m_Plane[ i ].y * vPoint.y + m_Plane[ i ].z * vPoint.z + m_Plane[ i ].w;
			if( fvalue < 0.0f ) //현재 정점이 6개의 평면중 한 평면의 외부에 있음
			{
				return false;
			}
		}

		//모든 평면의 양의 위치에 있을때 포함
		return true;
	}
}