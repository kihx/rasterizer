#include "stdafx.h"
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

	bool IsBackFaceSSE( const Vector4& v0, const Vector4& v1, const Vector4& v2 )
	{
		SSE::XXM128 v0SSE = SSE::XXM128_LoadUnaligned( v0.Value );
		SSE::XXM128 v1SSE = SSE::XXM128_LoadUnaligned( v1.Value );
		SSE::XXM128 v2SSE = SSE::XXM128_LoadUnaligned( v2.Value );

		SSE::XXM128 edge0 = SSE::XXM128_Subtract( v0SSE, v1SSE );		
		edge0 = SSE::XXM128_Normalize( edge0 );

		SSE::XXM128 edge1 = SSE::XXM128_Subtract( v0SSE, v2SSE );
		edge1 = SSE::XXM128_Normalize( edge1 );

		SSE::XXM128 normal = SSE::XXM128_CrossProduct( edge0, edge1 );

		// normal is not reversed because it is completely computed on SSE. 
		// So we should load ViewDir as Big-endian.
		const SSE::XXM128 ViewDir = SSE::XXM128_LoadReverse( 0.0f, 0.0f, 1.0f, 0.0f );	// in WVP space

		// Pass if this primitive is toward back.
		float toward = 0.0f;
		SSE::XXM128_Store( toward, SSE::XXM128_DotProduct( normal, ViewDir ) );

		if ( toward <= 0.0f )
		{
			return true;
		}

		return false;
	}
}
