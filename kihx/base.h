#pragma once

#include <assert.h>
#include <utility>
#include <memory>
#include <type_traits>

#pragma warning( disable: 4201 )	// warning C4201: nonstandard extension used : nameless struct/union


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


// macros
//
#define LOG_WARNING( msg )	{ printf( "[Warning] %s, in %s at %d\n", msg, __FUNCTION__, __LINE__ ); }


//------------Output FileName & LineNumber Show-----------------
#define __STR2(x) #x
#define __STR(x) __STR2(x)
#define __TODOMSG(desc) message(__FILE__ "(" __STR(__LINE__) "): TODO: " #desc)
#define __TODO(desc) __pragma(__TODOMSG(desc))
#define __UNDONEMSG(desc) message(__FILE__ "(" __STR(__LINE__) "): UNDONE: " #desc)
#define __UNDONE(desc) __pragma(__UNDONEMSG(desc))
//-------------------------------------------------------------- 


#define NONCOPYABLE_CLASS( ClassName )	\
	private:	\
		ClassName::ClassName( const ClassName& ) = delete;	\
		ClassName& operator=( const ClassName& ) = delete;


// utility functions
//
template<typename T>
inline const T& Min( const T& lhs, const T& rhs )
{
	static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point" );
	return (lhs <= rhs) ? lhs : rhs;
}

template<typename T>
inline const T& Max( const T& lhs, const T& rhs )
{
	static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point" );
	return ( lhs >= rhs ) ? lhs : rhs;
}

template<typename T>
inline const T& Clamp( const T& value, const T& min, const T& max )
{
	static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point" );
	return Max<T>( Min<T>( value, max ), min );
}

template<typename T>
inline void Swap( T& lhs, T& rhs, typename std::enable_if< std::is_scalar<T>::value >::type* = nullptr )
{
	// if T is scalar type, we do not use move semantics
	T tmp = lhs;
	lhs = rhs;
	rhs = tmp;
}

template<typename T>
inline void Swap( T& lhs, T& rhs, typename std::enable_if< !std::is_scalar<T>::value >::type* = nullptr )
{
	T tmp = std::move( lhs );
	lhs = std::move( rhs );
	rhs = std::move( tmp );
}

template<typename T1, typename T2>
inline T2 FloatToInteger( T1 f )
{
	static_assert( std::is_floating_point<T1>::value, "input type must be floating point" );
	static_assert( std::is_integral<T2>::value, "return type must be integer" );
	__UNDONE( change to a faster function );
	return static_cast< T2 >( floor( f ) );
}


// utility classes
//
template<class T>
class Singleton
{
public:
	template<typename... Args>
	static T* GetInstance( Args&&... args )
	{
		static_assert(
			std::is_class<T>::value &&
			!std::is_polymorphic<T>::value &&
			!std::is_pointer<T>::value,
			"invalid type of Singleton" );

		if ( s_instance == nullptr )
		{
			s_instance = new T( std::forward<Args>( args )... );
		}
	
		return s_instance;
	}

	static void DestroyInstance()
	{
		delete s_instance;
		s_instance = nullptr;
	}

private:
	static T* s_instance;
};

template <class T> 
T* Singleton<T>::s_instance = nullptr;



template<class T>
class LockGuard
{
public:
	LockGuard( std::shared_ptr<T> obj ) :
		m_obj( obj )
	{
		m_obj->Lock();
	}

	~LockGuard()
	{
		m_obj->Unlock();
	}

private:
	std::shared_ptr<T> m_obj;
};

template<class T>
class LockGuardPtr
{
public:
	LockGuardPtr( std::shared_ptr<T> obj ) :
		m_obj( obj )
	{
		m_obj->Lock( &m_p );
	}

	~LockGuardPtr()
	{
		m_obj->Unlock();
	}

	void* Ptr()
	{
		return m_p;
	}

private:
	std::shared_ptr<T> m_obj;
	void* m_p;
};

