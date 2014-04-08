#include "stdafx.h"
#include "threading.h"
#include "concommand.h"

//#define THREAD_CPP1x

#ifdef THREAD_CPP1x
#include <thread>
#else
#include <windows.h>
#include <process.h>
#endif


namespace kih
{
	void* f_beginthread(
		void *security,
		unsigned int stack_size,
		unsigned int( __stdcall *start_address )( void * ),
		void *arglist,
		unsigned int initflag,
		unsigned int *thrdaddr
		)
	{
		return reinterpret_cast< void* >(
			_beginthreadex( security,
			stack_size,
			start_address,
			arglist,
			initflag,
			thrdaddr
			) );
	}

	void f_endthreadex( unsigned int retval )
	{
		_endthreadex( retval );
	}

	unsigned int f_waitforsingleobject( void* hHandle, unsigned int dwMilliseconds )
	{
		return WaitForSingleObject( static_cast< HANDLE >( hHandle ), dwMilliseconds );
	}
};



#include <iostream>

DEFINE_COMMAND( thread_test )
{
	float yCaptured = 13.2f;

	Thread t
	{
		[=]( int i, const char* str )
		{
			std::cout.sync_with_stdio();
			std::cout << "i: " << i << std::endl;
			std::cout << "str: " << str << std::endl;
			std::cout << "y : " << yCaptured << std::endl;
		},
		5,
		"test"
	};
	t.Join();
}
