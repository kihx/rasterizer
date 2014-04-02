#pragma once

#include "base.h"


namespace kih
{
	class ReentryAssert
	{
		NONCOPYABLE_CLASS( ReentryAssert );

	public:
		explicit ReentryAssert( int& semephore ) :
			m_semaphore( ++semephore )
		{
		}

		~ReentryAssert()
		{
			--m_semaphore;
		}

		FORCEINLINE bool Verify( int limit ) const
		{
			return m_semaphore <= limit;
		}

	private:
		int& m_semaphore;
	};

#define REENTRY_ASSERT( limit )		static int __semephore = 0;	\
	ReentryAssert reentryAssert( __semephore );		\
	assert( reentryAssert.Verify( limit ) );
};
