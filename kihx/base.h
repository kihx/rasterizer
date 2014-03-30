#pragma once

#include "type.h"
#include <assert.h>
#include <utility>
#include <memory>
#include <type_traits>


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

	void* Ptr()
	{
		return m_ptr;
	}

private:
	std::shared_ptr<T> m_obj;
	void* m_ptr;
};

