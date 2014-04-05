#include "stdafx.h"
#include "concommand.h"

#include <vector>
#include <iostream>

namespace kih
{
	FORCEINLINE ConsoleVariable* ToConsoleVariable( ConsoleCommand* cmd )
	{
		return dynamic_cast< ConsoleVariable* >( cmd );
	}

	// String splitter: http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
	template<typename T>
	std::vector<T> SplitString( const T& str, const T& delimiters ) 
	{
		std::vector<T> v;
		T::size_type start = 0;
		auto pos = str.find_first_of( delimiters, start );
		while ( pos != T::npos ) 
		{
			if ( pos != start ) // ignore empty tokens
			{
				v.emplace_back( str, start, pos - start );
			}
			start = pos + 1;
			pos = str.find_first_of( delimiters, start );
		}

		if ( start < str.length() ) // ignore trailing delimiter
		{
			v.emplace_back( str, start, str.length() - start ); // add what's left of the string
		}
		return v;
	}


	/* class ConsoleCommand
	*/
	ConsoleCommand::ConsoleCommand( const std::string& name, ConsoleCommandCallback func ) :
		m_name( name ),
		m_func( func )
	{
		Assert( !m_name.empty() );
		ConsoleCommandExecuter::GetInstance()->AddCommand( this );
	}


	/* class ConsoleCommand
	*/
	ConsoleVariable::ConsoleVariable( const std::string& name, const std::string& value, ConsoleCommandCallback func ) :
		ConsoleCommand( name, func ),
		m_value( value )
	{
	}

	void ConsoleVariable::SetValue( const std::string& value )
	{
		if ( m_value == value )
		{
			return;
		}

		m_value = value;
		Call();
	}

	void ConsoleVariable::SetValue( std::string&& value )
	{
		if ( m_value == value )
		{
			return;
		}

		m_value = std::move( value );
		Call();
	}

	void ConsoleVariable::SetValue( int value )
	{
		if ( Int() == value )
		{
			return;
		}

		m_value = std::to_string( value );
		Call();
	}

	void ConsoleVariable::SetValue( float value )
	{
		if ( Float() == value )
		{
			return;
		}

		m_value = std::to_string( value );
		Call();
	}


	/* class ConsoleCommandExecuter
	*/
	void ConsoleCommandExecuter::Execute( const char* cmdString )
	{
		if ( cmdString == nullptr )
		{
			return;
		}

		VerifyReentry( 1 );

		std::vector<std::string> params = SplitString<std::string>( std::string( cmdString ), std::string( " " ) );
		if ( params.size() <= 0 )
		{
			return;
		}

		// We assume that
		// param 0: name
		// param 1: value
		ConsoleCommand* command = FindCommand( params[0] );
		if ( command == nullptr )
		{
			return;
		}
		
		// If the command has a callback, call this.
		if ( command->HasCallback() )
		{
			command->Call();
		}
		else	
		{
			// Otherwise, set or display its value if possible.
			if ( command->Type() != ConsoleCommandType::Variable )
			{
				return;
			}

			if ( ConsoleVariable* conVar = ToConsoleVariable( command ) )
			{
				// If there is a value parameter, change the value of the command.
				if ( params.size() >= 2 )
				{
					conVar->SetValue( params[1] );
				}
				else  // otherwise, display the current value of the command.
				{
					if ( !conVar->String().empty() )
					{
						std::cout << conVar->Name() << ": " << conVar->String() << std::endl;
					}
				}
			}
		}
	}

	ConsoleCommand* ConsoleCommandExecuter::FindCommand( const std::string& name )
	{
		auto iter = m_commandMap.find( name );
		if ( iter != m_commandMap.end() )
		{
			return iter->second;
		}

		return nullptr;
	}

	void ConsoleCommandExecuter::AddCommand( ConsoleCommand* command )
	{
		if ( command == nullptr )
		{
			return;
		}

		ConsoleCommand* oldCommand = FindCommand( command->Name() );
		if ( oldCommand )
		{
			LOG_WARNING( "cannot add an existed command" );
			return;
		}

		VerifyReentry( 1 );
		m_commandMap.insert( { command->Name(), command } );
	}

	void ConsoleCommandExecuter::Help()
	{
		for ( const auto& elem : m_commandMap )
		{
			if ( elem.second == nullptr )
			{
				VerifyNoEntry();
				continue;
			}

			switch ( elem.second->Type() )
			{
			case ConsoleCommandType::Command:
				std::cout << elem.first << std::endl;
				break;

			case ConsoleCommandType::Variable:
				if ( ConsoleVariable* conVar = ToConsoleVariable( elem.second ) )
				{
					std::cout << conVar->Name() << ": " << conVar->String() << std::endl;
				}
				break;

			default:
				VerifyNoEntry();
			}
		}
	}
};



/* cheats
*/
DEFINE_COMMAND( help )
{
	ConsoleCommandExecuter::GetInstance()->Help();
}

DEFINE_COMMAND( exit )
{
	exit( 0 );
}

