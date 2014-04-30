#include "stdafx.h"
#include "concommand.h"
#include "mathlib.h"
#include "stdsupport.h"

#include <iostream>


namespace kih
{
	// String splitter: http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
	template<class T>
	StlVector<T> SplitString( const T& str, const T& delimiters ) 
	{
		StlVector<T> v;
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
	ConsoleCommand::ConsoleCommand( const std::string& name, unsigned int flags, ConsoleCommandCallback func ) :
		m_flags( flags ),
		m_name( name ),
		m_func( func )
	{
		Assert( !m_name.empty() );
		ConsoleCommandExecuter::GetInstance()->AddCommand( this );
	}


	/* class ConsoleCommand
	*/
	ConsoleVariable::ConsoleVariable( const std::string& name, const std::string& value, unsigned int flags, ConsoleCommandCallback func ) :
		ConsoleCommand( name, flags, func )
	{
		SetValue( value );
	}

	void ConsoleVariable::SetValue( const std::string& value )
	{
		if ( m_value == value )
		{
			return;
		}

		m_int = std::stoi( value );
		m_float = std::stof( value );
		m_value = value;		
		Call();
	}

	void ConsoleVariable::SetValue( std::string&& value )
	{
		if ( m_value == value )
		{
			return;
		}

		m_int = std::stoi( value );
		m_float = std::stof( value );
		m_value = std::move( value );
		Call();
	}

	void ConsoleVariable::SetValue( int value )
	{
		if ( Int() == value )
		{
			return;
		}

		m_int = value;
		m_float = static_cast<float>( value );
		m_value = std::to_string( value );
		Call();
	}

	void ConsoleVariable::SetValue( float value )
	{
		if ( Float() == value )
		{
			return;
		}

		m_int = SSE::Trunc( value );
		m_float = value;
		m_value = std::to_string( value );
		Call();
	}


	/* class ConsoleCommandExecuter
	*/
	void ConsoleCommandExecuter::Execute( const char* cmdString ) const
	{
		if ( cmdString == nullptr )
		{
			return;
		}

		VerifyReentry();

		StlVector<std::string> params = SplitString<std::string>( std::string( cmdString ), std::string( " " ) );
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
			if ( command->IsCommand() )
			{
				return;
			}

			if ( ConsoleVariable* conVar = dynamic_cast< ConsoleVariable* >( command ) )
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

	ConsoleCommand* ConsoleCommandExecuter::FindCommand( const std::string& name ) const
	{
		auto iter = m_commandMap.find( name );
		if ( iter != m_commandMap.end() )
		{
			return iter->second;
		}

		return nullptr;
	}

	ConsoleVariable* ConsoleCommandExecuter::FindCommandVariable( const std::string& name ) const
	{
		auto iter = m_commandMap.find( name );
		if ( iter != m_commandMap.end() )
		{
			if ( ConsoleCommand* command = iter->second )
			{
				if ( !command->IsCommand() )
				{
					return dynamic_cast<ConsoleVariable*>( command );
				}
			}
		}

		return nullptr;
	}

	void ConsoleCommandExecuter::Help() const
	{
		for ( const auto& elem : m_commandMap )
		{
			ConsoleCommand* cmd = elem.second;
			if ( cmd == nullptr )
			{
				VerifyNoEntry();
				continue;
			}

			if ( cmd->IsCommand() )
			{
				std::cout << elem.first << std::endl;
			}
			else
			{
				if ( const ConsoleVariable* conVar = dynamic_cast< const ConsoleVariable* >( cmd ) )
				{
					std::cout << conVar->Name() << " " << conVar->String() << std::endl;
				}			
			}
		}
	}

	void ConsoleCommandExecuter::VerifyUnitTestAll() const
	{
		for ( const auto& elem : m_commandMap )
		{
			const ConsoleCommand* cmd = elem.second;
			if ( cmd == nullptr )
			{
				VerifyNoEntry();
				continue;
			}

			if ( !cmd->IsUnitTest() )
			{
				continue;
			}

			std::cout << "\n[" << cmd->Name() << "]" << std::endl;
			cmd->Call();
		}
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

		VerifyReentry();
		m_commandMap.insert( { command->Name(), command } );
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

DEFINE_COMMAND( tdd )
{
	ConsoleCommandExecuter::GetInstance()->VerifyUnitTestAll();
}
