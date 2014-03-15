#pragma once

template <typename T>
void Safe_Delete_VecList(T& p)
{
	T::iterator	iter = p.begin();

	while(iter != p.end())
	{
		Safe_Delete(*iter);
		++iter;
	}

	p.clear();
}

template <typename T>
void Safe_Delete(T& p)
{
	if(p)
	{
		delete	p;
		p	= nullptr;
	}
}

template <typename T>
void Safe_Delete_Array(T& p)
{
	if(p)
	{
		delete	[]	p;
		p	= nullptr;
	}
}

template <typename T>
void Safe_Release(T& p)
{
	if(p)
	{
		p->Release();
		p	= nullptr;
	}
}