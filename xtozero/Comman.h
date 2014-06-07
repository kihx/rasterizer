#ifndef _COMMAN_H_
#define _COMMAN_H_

#include <memory>

namespace xtozero
{
	template< typename T >
	void Swap( T& lhs, T& rhs )
	{
		T temp = lhs;
		lhs = rhs;
		rhs = temp;
	}

	template< typename T >
	class CSingletonBase
	{
	protected:
		CSingletonBase( ){}
		~CSingletonBase( ){}
	public:
		CSingletonBase( const CSingletonBase<T>& ) = delete;
		CSingletonBase<T>& operator=(const CSingletonBase<T>&) = delete;

		static T& GetInstance( )
		{
			static T instance;
			return instance;
		}
	};
}

#endif