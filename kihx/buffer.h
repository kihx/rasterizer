#pragma once

#include "base.h"
#include "matrix.h"
#include "vector.h"


namespace kih
{
	/* class ConstantBuffer
	*/
	class ConstantBuffer
	{
	public:
		static const int Capacity = 128;

		enum NamedIndex
		{
			#define	ENUM_MATRIX(e)	( e + 4 )
			#define	ENUM_VECTOR(e)	( e + 1 )

			WorldMatrix				= 0,
			ViewMatrix				= ENUM_MATRIX( WorldMatrix ),
			ProjectionMatrix		= ENUM_MATRIX( ViewMatrix ),
			WVPMatrix				= ENUM_MATRIX( ProjectionMatrix ),
			DiffuseColor			= ENUM_MATRIX( WVPMatrix ),
		};

		ConstantBuffer() = default;

		FORCEINLINE const Vector4& GetVector4( int index ) const
		{
			Assert( index >= 0 && index < Capacity );
			return m_constantBuffer[index];
		}

		FORCEINLINE const Matrix4& GetMatrix4( int index ) const
		{
			Assert( index >= 0 && index < Capacity );
			// FIXME: is safe such type casting?
			return *( reinterpret_cast< const Matrix4* >( &m_constantBuffer[index] ) );
		}

		FORCEINLINE void SetFloat4( int index, const float* value )
		{
			Assert( index >= 0 && index < Capacity );
			m_constantBuffer[index].X = value[0];
			m_constantBuffer[index].Y = value[1];
			m_constantBuffer[index].Z = value[2];
			m_constantBuffer[index].W = value[3];
		}

		FORCEINLINE void SetVector4( int index, const Vector4& value )
		{
			Assert( index >= 0 && index < Capacity );
			m_constantBuffer[index] = value;
		}

		FORCEINLINE void SetMatrix4( int index, const Matrix4& value )
		{
			SetFloat4( index, value.A[0] );
			SetFloat4( index + 1, value.A[1] );
			SetFloat4( index + 2, value.A[2] );
			SetFloat4( index + 3, value.A[3] );
		}

	private:
		Vector4 m_constantBuffer[Capacity];
	};

};

using kih::ConstantBuffer;
