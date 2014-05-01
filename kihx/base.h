#pragma once

#include "assertion.h"
#include "type.h"
#include "memory.h"

#include <functional>
#include <type_traits>
#include <memory>
#include <utility>


#pragma warning( disable: 4127 )	// warning C4127: conditional expression is constant


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


#define NONCOPYABLE_STRUCT( StructName )	\
	StructName::StructName( const StructName& ) = delete;	\
	StructName& operator=( const StructName& ) = delete;

#define NONCOPYABLE_CLASS( ClassName )	\
	private:	\
		NONCOPYABLE_STRUCT( ClassName )


namespace kih
{
	// utility functions
	//
	template<class T>
	FORCEINLINE void Swap( T& lhs, T& rhs, typename std::enable_if< std::is_scalar<T>::value >::type* = nullptr )
	{
		// if T is scalar type, we do not use move semantics
		T tmp = lhs;
		lhs = rhs;
		rhs = tmp;
	}

	template<class T>
	FORCEINLINE void Swap( T& lhs, T& rhs, typename std::enable_if< !std::is_scalar<T>::value >::type* = nullptr )
	{
		T tmp = std::move( lhs );
		lhs = std::move( rhs );
		rhs = std::move( tmp );
	}

	// http://stackoverflow.com/questions/1500363/compile-time-sizeof-array-without-using-a-macro
	template<class T, size_t N>
	char( &_SizeOfArray( const T( &a )[N] ) )[N];
	// VS 2013 does not support constexpr
	//template<class... Args>
	//constexpr size_t SizeOfArray( Args&&... args )
	//{
	//	return sizeof( _SizeOfArray( args ) );
	//}
	#define SIZEOF_ARRAY( a )	( sizeof( _SizeOfArray( a ) ) )

	
	template<class T>
	FORCEINLINE T AlignSize( T size, T alignment )
	{
		static_assert( std::is_integral<T>::value, "should use integal type arguments" );
		T alignBase = alignment - static_cast<T>( 1 );
		return ( ( size + alignBase ) & ~alignBase );
	}


	// utility classes
	//

	/* Unused parameters
	*/
	template<class... Args> 
	FORCEINLINE void Unused( Args&&... )
	{}

	/* struct LoopUnroll
	*/
	template<int N>
	struct LoopUnroll
	{
		template<class Func>
		static void Work( Func func )
		{			
			func();
			LoopUnroll< N - 1 >::Work( func );
		}
	};

	template<>
	struct LoopUnroll<1>
	{
		template<class Func>
		static void Work( Func func )
		{
			func();
		}
	};


	/* class Singleton
	*/
	template<class T>
	class Singleton
	{
	public:
		template<class... Args>
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


	/* class LockGuard
	*/
	template<class T>
	class LockGuard final
	{
		NONCOPYABLE_CLASS( LockGuard );

	public:
		explicit LockGuard( T& obj ) :
			m_obj( obj )
		{
			m_obj.Lock();
		}

		~LockGuard()
		{
			m_obj.Unlock();
		}

	private:
		T& m_obj;
	};


	/* class LockGuardPtr
	*/
	template<class T>
	class LockGuardPtr final
	{
		NONCOPYABLE_CLASS( LockGuardPtr );

	public:
		explicit LockGuardPtr( std::shared_ptr<T> obj ) :
			m_obj( obj ),
			m_ptr( nullptr )
		{
			if ( m_obj )
			{
				m_obj->Lock( &m_ptr );
			}
		}

		~LockGuardPtr()
		{
			if ( m_obj )
			{
				m_obj->Unlock();
			}
		}

		FORCEINLINE void* Ptr()
		{
			return m_ptr;
		}

	private:
		std::shared_ptr<T> m_obj;
		void* m_ptr;
	};
};

using kih::LoopUnroll;
using kih::Singleton;
using kih::LockGuard;
using kih::LockGuardPtr;

