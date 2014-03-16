#pragma once

#include <assert.h>
#include <utility>
#include <memory>


#define LOG_WARNING( msg )	{ printf( "[Warning] %s, in %s at %d\n", msg, __FUNCTION__, __LINE__ ); }


//------------Output FileName & LineNumber Show-----------------
#define __STR2(x) #x
#define __STR(x) __STR2(x)
#define __TODOMSG(desc) message(__FILE__ "(" __STR(__LINE__) "): TODO: " #desc)
#define __TODO(desc) __pragma(__TODOMSG(desc))
#define __UNDONEMSG(desc) message(__FILE__ "(" __STR(__LINE__) "): UNDONE: " #desc)
#define __UNDONE(desc) __pragma(__UNDONEMSG(desc))
//-------------------------------------------------------------- 


class Uncopyable
{
protected:
	Uncopyable() {}
	~Uncopyable() {}

private:
	Uncopyable( const Uncopyable& );
	Uncopyable& operator=( const Uncopyable& );
};



template<class T>
class Singleton
{
protected:
	Singleton()
	{
		static_assert( !std::is_pointer<T>::value && std::is_class<T>::value, "invalid class type of Singleton" );
	}

public:
	//template<typename... Args>
	//static T* GetInstance( Args... args )
	//{
	//	if ( s_instance == NULL )
	//	{
	//		s_instance = std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
	//	}
	//
	//	return s_instance.get();
	//}

	static T* GetInstance()
	{
		if ( s_instance == NULL )
		{
			s_instance = std::unique_ptr<T>( new T() );
		}

		return s_instance.get();
	}

	static void DestroyInstance()
	{
		delete s_instance;
		s_instance = NULL;
	}

private:
	static std::unique_ptr<T> s_instance;
};

template <class T> 
std::unique_ptr<T> Singleton<T>::s_instance = NULL;

