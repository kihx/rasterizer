#ifndef _COMMAN_H_
#define _COMMAN_H_

namespace xtozero
{
	template< typename T >
	void Swap( T& lhs, T& rhs )
	{
		T temp = lhs;
		lhs = rhs;
		rhs = temp;
	}
}

#endif