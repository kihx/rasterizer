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
	private:
		static T* m_instance;
	public:
		CSingletonBase( const CSingletonBase<T>& ) = delete;
		CSingletonBase<T>& operator=(const CSingletonBase<T>&) = delete;

		static T* GetInstance( )
		{
			if ( m_instance == nullptr )
			{
				m_instance = new T();
			}

			return m_instance;
		}
		void ReleaseInstance()
		{
			delete m_instance;
			m_instance = nullptr;
		}
		~CSingletonBase(){}
	protected:
		CSingletonBase(){}
	};

	template< typename T >
	T* CSingletonBase<T>::m_instance = nullptr;
}

#endif