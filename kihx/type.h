#pragma once

#include <type_traits>


#pragma warning( disable: 4201 )	// warning C4201: nonstandard extension used : nameless struct/union



namespace kih
{
	typedef unsigned char byte;


	/* struct Color4
	*/
	template<typename T>
	struct Color4
	{
		static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point" );

		union
		{
			struct
			{
				T R;
				T G;
				T B;
				T A;
			};

			T Value[4];
		};

		Color4() :
			R( T() ),
			G( T() ),
			B( T() ),
			A( T() )
		{
		}

		Color4( T r, T g, T b, T a ) :
			R( r ),
			G( g ),
			B( b ),
			A( a )
		{
		}

		Color4( const T color[4] ) :
			R( color[0] ),
			G( color[1] ),
			B( color[2] ),
			A( color[3] )
		{
		}

		Color4( const Color4& c ) = default;
	};

	typedef Color4<byte> Color32;
	typedef Color4<float> Color128;



};

using kih::Color32;
using kih::Color128;
