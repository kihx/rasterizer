#pragma once
#include "..\Data\CoolD_Singleton.h"
#include "..\Data\CoolD_Type.h"

namespace CoolD
{	
	class Command;
	class ConsoleManager : public CSingleton<ConsoleManager>
	{
		friend class CSingleton<ConsoleManager>;
	private:
		ConsoleManager() = default;
		ConsoleManager(const ConsoleManager&) = delete;
		ConsoleManager& operator=(const ConsoleManager&) = delete;

	public:
		~ConsoleManager();

	public:
		void CommandExecute(const Dchar* cmd);
		void AddCommand(Command* command);
		void ShowCommand(const string& name);
		Command* FindCommand(const string& name) const;

	private:
		template<typename T>
		vector<T> SpliteCommand(const T& str, const T& delimiters);

	private:
		map<string, Command*> m_mapCommand;
	};

	template<class T>
	vector<T> ConsoleManager::SpliteCommand(const T& str, const T& delimiters)
	{
		vector<T> v;
		T::size_type start = 0;
		auto pos = str.find_first_of(delimiters, start);
		while( pos != T::npos )
		{
			if( pos != start ) // ignore empty tokens
			{
				v.emplace_back(str, start, pos - start);
			}
			start = pos + 1;
			pos = str.find_first_of(delimiters, start);
		}

		if( start < str.length() ) // ignore trailing delimiter
		{
			v.emplace_back(str, start, str.length() - start); // add what's left of the string
		}
		return v;
	}
};

