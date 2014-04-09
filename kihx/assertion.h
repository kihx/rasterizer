#pragma once

// TODO: write a custom assertion
#include <assert.h>


#ifdef _DEBUG
#define CODE_VERIFIER
#endif


#ifdef CODE_VERIFIER

#define Assert( exp )		assert( exp )

namespace kih
{
	class ReentryGuard
	{
	public:
		explicit ReentryGuard( unsigned int& semephore ) :
			m_semaphore( semephore )
		{
			Assert( Verify() );
			++m_semaphore;
		}

		ReentryGuard( const ReentryGuard& ) = delete;

		~ReentryGuard()
		{
			--m_semaphore;
		}

		FORCEINLINE bool Verify() const
		{
			return m_semaphore <= 0;
		}

		ReentryGuard& operator=( const ReentryGuard& ) = delete;

	private:
		unsigned int& m_semaphore;
	};
};

// Verify that a forbidden code path is reached.
#define VerifyNoEntry()				{ Assert( 0 && "this code path should never be called!" ); }

// Verify that a code path is reached once.
#define VerifyEntryOnce()			{ static bool __semephore__ = false; \
	Assert( !__semephore__ && "this code path should not be called more than once" ); \
	__semephore__ = true; }

// Verify that a code path is concurrently reached limit or not.
#define VerifyReentry()		static unsigned int __semephore__ = 0;	\
	kih::ReentryGuard reentryGuard( __semephore__ );
#else

#define Assert( exp )

#define VerifyNoEntry()
#define VerifyEntryOnce()
#define VerifyReentry()

#endif	// #ifdef CODE_VERIFIER
