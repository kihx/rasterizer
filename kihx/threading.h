#pragma once

#include "base.h"

#include <stdexcept>


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
	void f_endthreadex( unsigned int retval );
	unsigned int f_waitforsingleobject( void* hHandle, unsigned int dwMilliseconds );


	/* class Thread
	*/
	class Thread
	{
		NONCOPYABLE_CLASS( Thread );

	public:
		Thread() :
			m_handle( nullptr ),
			m_id( 0 ),
			m_started( false )
		{
		}

		Thread( Thread&& other )
		{
			m_handle = other.m_handle;
			m_id = other.m_id;
			m_func = other.m_func;
			m_started = other.m_started;

			MakeNull( other );
		}

		template<typename Func, typename... Args>
		explicit Thread( Func&& func, Args&&... args )
		{
			auto f = std::bind( std::forward<Func>( func ), std::forward<Args>( args )... );
			m_func = f;
			m_started = false;
			m_handle = f_beginthread( 
				nullptr,
				0,	// stack_size
				[]( void* arg ) -> unsigned int		// thread func
				{
					if ( Thread* t = reinterpret_cast< Thread* >( arg ) )
					{
						if ( t->m_func )
						{
							t->m_started = true;
							t->m_func();
						}
					}
					//f_endthreadex( 0 );
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

			// Wait until this thread is started.
			while ( !m_started );
		}

		~Thread()
		{
			if ( Joinable() )
			{
				std::terminate();
			}
		}

		FORCEINLINE void* Handle()
		{
			return m_handle;
		}

		FORCEINLINE unsigned int ID()
		{
			return m_id;
		}

		FORCEINLINE bool Joinable() const
		{
			return m_handle != nullptr;
		}

		FORCEINLINE void Join()
		{
			//if ( m_id == Current.m_id )
			//{
			//	throw std::runtime_error( "deadlock would occur" );
			//}
			if ( !Joinable() )
			{
				throw std::runtime_error( "this thread cannot join" );
			}

			Join( m_handle );
			MakeNull( *this );
		}

		static FORCEINLINE void Join( void* handle )
		{
			Assert( handle );
			f_waitforsingleobject( handle, 0xFFFFFFFF );
		}

		//static Thread& Current()
		//{
		//	return *this;
		//}

		FORCEINLINE void Swap( Thread& other )
		{
			kih::Swap( *this, other );
		}

		// move assignment
		FORCEINLINE Thread& operator=( Thread&& other )
		{
			m_handle = other.m_handle;
			m_id = other.m_id;
			m_func = other.m_func;
			m_started = other.m_started;

			MakeNull( other );

			return *this;
		}

		// c++11 compatible
		FORCEINLINE void join()
		{
			Join();
		}

		FORCEINLINE void swap( Thread& other )
		{
			Swap( other );
		}

	private:
		static FORCEINLINE void MakeNull( Thread& t )
		{
			t.m_handle = nullptr;
			t.m_id = 0;
			t.m_func = nullptr;
			t.m_started = false;
		}

	private:
		void* m_handle;
		unsigned int m_id;
		std::function<void()> m_func;
		volatile bool m_started;
	};

	
};

using kih::Thread;
