#pragma once

#include "base.h"

#include <functional>

namespace kih
{
	// Wrapper functions for threading.
	// Do NOT directly call them. Use threading classes.
	void* f_beginthread(
		void* security,
		unsigned int stack_size,
		unsigned int( __stdcall* start_address )( void* ),
		void* arglist,
		unsigned int initflag,
		unsigned int* thrdaddr
		);
	unsigned int f_waitforsingleobject( void* hHandle, unsigned int dwMilliseconds );


	/* class Thread
	*/
	class Thread
	{
		NONCOPYABLE_CLASS( Thread );

	public:
		template<typename Func, typename... Args>
		explicit Thread( Func&& func, Args&&... args )
		{
			auto f = std::bind( std::forward<Func>( func ), std::forward<Args>( args )... );
			m_func = f;
			
			m_handle = f_beginthread( 
				nullptr,
				0,	// stack_size
				[]( void* arg ) -> unsigned int		// thread func
				{
					if ( Thread* t = reinterpret_cast< Thread* >( arg ) )
					{
						if ( t->m_func )
						{
							t->m_func();
						}
					}
					return 0;
				},
				this,	// arg
				0,
				&m_id );

			if ( m_handle == nullptr )
			{
				Assert( 0 && "threading error" );
				//exit( 1 );
			}
		}

		void Join()
		{
			//if ( m_id == Current.m_id )
			//{
			//	throw std::runtime_error( "deadlock would occur" );
			//}
			f_waitforsingleobject( m_handle, 0xFFFFFFFF );
		}

		//static Thread& Current()
		//{
		//	return *this;
		//}

	private:
		void* m_handle;
		unsigned int m_id;
		std::function<void()> m_func;
	};

	
};

using kih::Thread;
