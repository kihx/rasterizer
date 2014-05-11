#pragma once
#include <memory>
#include <mutex>

namespace CoolD
{
	//thread_safe singleton
	template <typename T>
	class CSingleton
	{
	protected:
		CSingleton() = default;
		CSingleton(const CSingleton<T>& src) = default;
		CSingleton<T>& operator=(const CSingleton<T>& rhs) = default;

	public:
		static T& GetInstance()
		{
			std::call_once(m_onceFlag,	[] 
										{
											m_instance.reset(new T);
										});

			return *m_instance.get();
		}		

	private:
		static std::unique_ptr<T> m_instance;
		static std::once_flag m_onceFlag;
	};

	template <typename T>
	std::unique_ptr<T> CSingleton<T>::m_instance;

	template <typename T>
	std::once_flag CSingleton<T>::m_onceFlag;
}