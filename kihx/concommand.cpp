#include "stdafx.h"
#include "concommand.h"

#include <vector>
#include <iostream>

namespace kih
{
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
	ConsoleCommand::ConsoleCommand( const std::string& name, const std::string& value, ConsoleCommandCallback func ) :
		m_name( name ),
		m_value( value ),
		m_func( func )
	{
		Assert( !m_name.empty() );
		ConsoleCommandExecuter::GetInstance()->AddCommand( this );
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
		else	// otherwise, set or display its value.
		{
			// If there is a value parameter, change the value of the command.
			if ( params.size() >= 2 )
			{
				command->SetValue( params[1] );
			}
			else  // otherwise, display the current value of the command.
			{
				if ( !command->String().empty() )
				{
					std::cout << command->Name() << ": " << command->String() << std::endl;
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
};


DEFINE_COMMAND( exit )
{
	exit( 0 );
}

