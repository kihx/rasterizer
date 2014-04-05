#pragma once

#include "base.h"

#include <string>
#include <unordered_map>
#include <functional>


namespace kih
{
	class ConsoleCommandExecuter;

	/* class ConsoleCommand
	*/
	class ConsoleCommand
	{
		NONCOPYABLE_CLASS( ConsoleCommand );

		typedef void( *ConsoleCommandCallback )( );

	public:
		explicit ConsoleCommand( const std::string& name, const std::string& value, ConsoleCommandCallback func = nullptr );

		FORCEINLINE const std::string& Name() const
		{
			return m_name;
		}

		FORCEINLINE const std::string& String() const
		{
			return m_value;
		}

		FORCEINLINE int Int() const
		{			
			return std::stoi( m_value );
		}

		FORCEINLINE float Float() const
		{
			return std::stof( m_value );
		}

		FORCEINLINE void SetValue( const std::string& value )
		{
			m_value = value;
		}

		FORCEINLINE void SetValue( std::string&& value )
		{
			m_value = std::move( value );
		}

		FORCEINLINE void SetValue( int value )
		{
			m_value = std::to_string( value );
		}

		FORCEINLINE void SetValue( float value )
		{
			m_value = std::to_string( value );
		}

	private:
		FORCEINLINE bool HasCallback() const
		{
			return m_func != nullptr;
		}

		FORCEINLINE void Call()
		{
			if ( HasCallback() )
			{
				m_func();
			}
		}

	private:
		std::string m_name;
		std::string m_value;
		ConsoleCommandCallback m_func;

		friend class ConsoleCommandExecuter;
	};


	/* class CommandExecuter
	*/
	class ConsoleCommandExecuter final : public Singleton<ConsoleCommandExecuter>
	{
		friend class Singleton<ConsoleCommandExecuter>;

		NONCOPYABLE_CLASS( ConsoleCommandExecuter );

	private:
		ConsoleCommandExecuter() = default;

	public:
		~ConsoleCommandExecuter() = default;

		void Execute( const char* cmdString );

		ConsoleCommand* FindCommand( const std::string& name );

	private:
		void AddCommand( ConsoleCommand* command );

	private:
		std::unordered_map<std::string, ConsoleCommand*> m_commandMap;

		friend class ConsoleCommand;
	};
};

using kih::ConsoleCommand;
using kih::ConsoleCommandExecuter;


#define DEFINE_COMMAND( name )	\
	void __callback_##name();	\
	static ConsoleCommand __concommand_##name( #name, "", __callback_##name );	\
	void __callback_##name()
