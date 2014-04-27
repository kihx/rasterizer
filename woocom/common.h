#pragma once

#include <vector>
#include <algorithm>

#include <Windows.h>

struct PixelInfo
{
	PixelInfo(int x, const unsigned char* rgb) :m_x(x), m_z(0.0f)
	{
		m_rgb[0] = rgb[0];
		m_rgb[1] = rgb[1];
		m_rgb[2] = rgb[2];
	}
	PixelInfo(int x, float z, const unsigned char* rgb) : m_x(x), m_z(z)
	{
		m_rgb[0] = rgb[0];
		m_rgb[1] = rgb[1];
		m_rgb[2] = rgb[2];
	}

	bool operator < (const PixelInfo& rhs)
	{
		return m_x < rhs.m_x;
	}

	int m_x;
	float m_z;
	unsigned char m_rgb[3];
};

struct EdgeInfo
{
	EdgeInfo(){ m_edgeData.reserve(3); }
	EdgeInfo(int x, const unsigned char* rgb)
	{
		m_edgeData.emplace_back(x, rgb);
	}
	EdgeInfo(int x, float z, const unsigned char* rgb)
	{
		m_edgeData.emplace_back(x, z, rgb);
	}

	void Insert(int x, const unsigned char* rgb)
	{
		m_edgeData.emplace_back(x, rgb);
	}
	void Insert(int x, float z, const unsigned char* rgb)
	{
		m_edgeData.emplace_back(x, z, rgb);
	}
	void Sort()
	{
		if (m_edgeData.size() == 0)
		{
			return;
		}

		std::sort(m_edgeData.begin(), m_edgeData.end());

		if (m_edgeData.size() & 1)
		{
			Adjust();
		}
	}

	// 중복되어 들어간 홀수 edgData 제거
	void Adjust()
	{
		size_t num = m_edgeData.size();
		for (size_t i = 0; i + 1 < num; i += 2)
		{
			// 중복된 데이터 밀어당긴 후 pop
			if (m_edgeData[i].m_x == m_edgeData[i + 1].m_x)
			{
				m_edgeData[i + 1] = m_edgeData[i + 2];
			}
		}
		m_edgeData.pop_back();
	}

	std::vector<PixelInfo> m_edgeData;
};

class WMutex
{
public:
	WMutex(CRITICAL_SECTION* cs) : m_cs(cs)
	{
		EnterCriticalSection(m_cs);
	}
	~WMutex()
	{
		LeaveCriticalSection(m_cs);
	}
private:
	CRITICAL_SECTION*	m_cs;
};

template< typename T>
class Singleton
{
protected:
	Singleton(){}
	~Singleton(){}

	static T* m_Instance;

public:
	static T* GetInstance()
	{
		if (m_Instance == nullptr)
		{
			m_Instance = new T();
		}
		return m_Instance;
	}

	static void Destory()
	{
		if (m_Instance)
		{
			delete m_Instance;
			m_Instance = nullptr;
		}
	}
};

template< typename T >
T* Singleton<T>::m_Instance = nullptr;