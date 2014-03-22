#pragma once

#include <assert.h>
#include <utility>
#include <memory>


typedef unsigned char byte;


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


template<typename T>
inline const T& Min( const T& lhs, const T& rhs )
{
	return (lhs <= rhs) ? lhs : rhs;
}

template<typename T>
inline const T& Max( const T& lhs, const T& rhs )
{
	return ( lhs >= rhs ) ? lhs : rhs;
}

template<typename T>
inline const T& Clamp( const T& value, const T& min, const T& max )
{
	return Max<T>( Min<T>( value, max ), min );
}

template<typename T>
inline void Swap( T& lhs, T& rhs )
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
			"invalid type of Singleton");

		if ( s_instance == nullptr )
		{
			s_instance = std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
		}
	
		return s_instance.get();
	}

	static void DestroyInstance()
	{
		delete s_instance;
		s_instance = nullptr;
	}

private:
	static std::unique_ptr<T> s_instance;
};

template <class T> 
std::unique_ptr<T> Singleton<T>::s_instance = nullptr;



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

