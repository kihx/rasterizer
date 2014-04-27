#pragma once

#include "base.h"

#include <string>
#include <unordered_map>


namespace kih
{
	class ConsoleCommand;
	class ConsoleVariable;
	class ConsoleCommandExecuter;

	typedef void( *ConsoleCommandCallback )( );
		

	/* class ConsoleCommand
	*/
	class ConsoleCommand
	{
		NONCOPYABLE_CLASS( ConsoleCommand );

	public:
		enum PropertyFlags
		{
			FL_NONE = 0x0,
			FL_UNITTEST = 0x2,
		};

		ConsoleCommand() = default;
		ConsoleCommand( const std::string& name, unsigned int flags = FL_NONE, ConsoleCommandCallback func = nullptr );
		
		virtual ~ConsoleCommand() 
		{
		}

		virtual bool IsCommand() const
		{
			return true;
		}

		FORCEINLINE const std::string& Name() const
		{
			return m_name;
		}

		FORCEINLINE bool IsUnitTest() const
		{
			return ( m_flags & FL_UNITTEST ) != 0;
		}

	protected:
		FORCEINLINE bool HasCallback() const
		{
			return m_func != nullptr;
		}

		FORCEINLINE void Call() const
		{
			if ( HasCallback() )
			{
				m_func();
			}
		}

	private:
		unsigned int m_flags;
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
		explicit ConsoleVariable( const std::string& name, const std::string& value, unsigned int flags = FL_NONE, ConsoleCommandCallback func = nullptr );

		virtual bool IsCommand() const
		{
			return false;
		}

		FORCEINLINE const std::string& String() const
		{
			return m_value;
		}

		FORCEINLINE int Int() const
		{			
			return m_int;
		}

		FORCEINLINE bool Bool() const
		{
			return Int() != 0;
		}

		FORCEINLINE float Float() const
		{
			return m_float;
		}

		void SetValue( const std::string& value );
		void SetValue( std::string&& value );
		void SetValue( int value );
		void SetValue( float value );

	private:
		int m_int;
		float m_float;
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

		void Execute( const char* cmdString ) const;

		ConsoleCommand* FindCommand( const std::string& name ) const;
		ConsoleVariable* FindCommandVariable( const std::string& name ) const;

		void Help() const;

		void VerifyUnitTestAll() const;

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
	static ConsoleCommand __concommand_##name( #name, 0, __callback_##name );	\
	void __callback_##name()

#define DEFINE_UNITTEST( name )	\
	void __callback_##name();	\
	static ConsoleCommand __concommand_##name( #name, ConsoleCommand::FL_UNITTEST, __callback_##name );	\
	void __callback_##name()

