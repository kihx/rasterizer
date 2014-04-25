#include "stdafx.h"
#include "concommand.h"
#include "mathlib.h"


namespace kih
{
	bool IsBackFace( const Vector4& v0, const Vector4& v1, const Vector4& v2 )
	{
		Vector3 edge0 = Vector4( v0 - v1 ).Value;
		edge0.NormalizeInPlace();

		Vector3 edge1 = Vector4( v0 - v2 ).Value;
		edge1.NormalizeInPlace();

		Vector3 normal = edge0.CrossProduct( edge1 );

		// Pass if this primitive is toward back.
		const Vector3 ViewDir( 0.0f, 0.0f, 1.0f );	// in WVP space
		if ( ViewDir.DotProduct( normal ) <= 0.0f )
		{
			return true;
		}

		return false;
	}

	namespace SSE
	{
		bool IsBackFace( const Vector4& v0, const Vector4& v1, const Vector4& v2 )
		{
			xxm128 v0xxm = SSE::LoadUnaligned( v0.Value );
			xxm128 v1xxm = SSE::LoadUnaligned( v1.Value );
			xxm128 v2xxm = SSE::LoadUnaligned( v2.Value );

			return SSE::IsBackFace( v0xxm, v1xxm, v2xxm );
		}

		bool IsBackFace( const xxm128& v0, const xxm128& v1, const xxm128& v2 )
		{
			xxm128 edge0 = SSE::Subtract( v0, v1 );
			edge0 = SSE::Normalize( edge0 );

			xxm128 edge1 = SSE::Subtract( v0, v2 );
			edge1 = SSE::Normalize( edge1 );

			xxm128 normal = SSE::CrossProduct( edge0, edge1 );

			// normal is not reversed because it is completely computed on SSE. 
			// So we should load ViewDir as Big-endian.
			const xxm128 ViewDir = SSE::LoadReverse( 0.0f, 0.0f, 1.0f, 0.0f );	// in WVP space

			// Pass if this primitive is toward back.
			if ( SSE::LessEqual( SSE::DotProduct( normal, ViewDir ), SSE::xxm128_Zero ).m128_i32[3] == 0xFFFFFFFF )
			{
				return true;
			}

			return false;
		}
	}
}




DEFINE_UNITTEST( sse_test )
{
	using namespace kih;

	xxm128 v0 = SSE::LoadReverse( 1.1f, 1.5f, 1.8f, 2.0f );
	xxm128 v0Round = SSE::Round( v0 );
	SSE::Equal( v0, v0Round ).m128_i32;
}
