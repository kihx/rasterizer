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
void Safe_Delete_Map(T& p)
{
	T::iterator iter = p.begin();
	while( iter != p.end() )
	{
		Safe_Delete(iter->second);
		++iter;
	}
	p.clear();
}

template <typename T>
void Safe_Delete_ListInMap(T& p)
{
	T::iterator iter = p.begin();
	while( iter != p.end() )
	{
		Safe_Delete_VecList(iter->second);		
		++iter;
	}
	p.clear();
}
