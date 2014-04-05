#pragma once

#include "base.h"

#include <string>
#include <unordered_map>
#include <functional>


namespace kih
{
	class ConsoleCommand;
	class ConsoleVariable;
	class ConsoleCommandExecuter;

	typedef void( *ConsoleCommandCallback )( );

	
	enum class ConsoleCommandType
	{
		Command,
		Variable
	};


	/* class ConsoleCommand
	*/
	class ConsoleCommand
	{
		NONCOPYABLE_CLASS( ConsoleCommand );

	public:
		explicit ConsoleCommand( const std::string& name, ConsoleCommandCallback func = nullptr );

		virtual ConsoleCommandType Type() const
		{
			return ConsoleCommandType::Command;
		}

		FORCEINLINE const std::string& Name() const
		{
			return m_name;
		}

	protected:
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
		ConsoleCommandCallback m_func;

		friend class ConsoleCommandExecuter;
	};


	/* class ConsoleVariable
	*/
	class ConsoleVariable : public ConsoleCommand
	{
		NONCOPYABLE_CLASS( ConsoleVariable );

	public:
		explicit ConsoleVariable( const std::string& name, const std::string& value, ConsoleCommandCallback func = nullptr );

		virtual ConsoleCommandType Type() const
		{
			return ConsoleCommandType::Variable;
		}

		FORCEINLINE const std::string& String() const
		{
			return m_value;
		}

		FORCEINLINE int Int() const
		{			
			return std::stoi( m_value );
		}

		FORCEINLINE bool Bool() const
		{
			return Int() != 0;
		}

		FORCEINLINE float Float() const
		{
			return std::stof( m_value );
		}

		void SetValue( const std::string& value );
		void SetValue( std::string&& value );
		void SetValue( int value );
		void SetValue( float value );

	private:
		std::string m_value;
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

		void Help();

	private:
		void AddCommand( ConsoleCommand* command );

	private:
		std::unordered_map<std::string, ConsoleCommand*> m_commandMap;

		friend class ConsoleCommand;
	};
};

using kih::ConsoleCommand;
using kih::ConsoleVariable;
using kih::ConsoleCommandExecuter;


#define DEFINE_COMMAND( name )	\
	void __callback_##name();	\
	static ConsoleCommand __concommand_##name( #name, __callback_##name );	\
	void __callback_##name()
