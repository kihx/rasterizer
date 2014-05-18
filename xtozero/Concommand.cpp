#include "stdafx.h"
#include "Concommand.h"

#include <iostream>

namespace cmd
{
	const int CTokenizer::ArgC( ) const
	{
		return m_argv.size( );
	}

	const std::string& CTokenizer::ArgV( int index ) const
	{
		if ( index >= ArgC() )
		{
			index = ArgC() - 1;
		}

		return m_argv[index];
	}

	void CTokenizer::DoTokenizing( const char* str, const char token )
	{
		m_argv.clear();

		std::string string( str );

		for ( ; ; )
		{
			unsigned int pos = string.find( token );

			if ( pos == std::string::npos ) // bad position
			{
				m_argv.emplace_back( string.substr( 0, string.length() ) );
				return;
			}
			else
			{
				m_argv.emplace_back( string.substr( 0, pos ) );
				string = string.substr( pos + 1, string.length() - pos );
			}
		}
	}

	CConvar::CConvar( const char* name, const char* value ) : m_name( name ), m_value(value)
	{
		CConcommandExecutor::GetInstance( )->AddConvar( m_name, this );
		m_float = static_cast<float>(atof( m_value.c_str( ) ) );
		m_int = static_cast<int>(m_float);
		m_bool = ( m_int > 0 ) ? true : false;
	}

	const std::string& CConvar::GetName( ) const
	{
		return m_name;
	}
	const int CConvar::GetInt() const
	{
		return m_int;
	}
	const float CConvar::GetFloat( ) const
	{
		return m_float;
	}
	const bool CConvar::GetBool() const
	{
		return m_bool;
	}
	const char* CConvar::GetChar() const
	{
		return m_value.c_str();
	}

	void CConvar::SetValue( const char* value )
	{
		m_value = std::string( value );
		m_float = static_cast<float>( atof( m_value.c_str( ) ) );
		m_int = static_cast<int>(m_float);
		m_bool = (m_int > 0) ? true : false;
	}

	void CConvar::SetValue( const std::string& value )
	{
		m_value = value;
		m_float = static_cast<float>(atof( m_value.c_str( ) ));
		m_int = static_cast<int>(m_float);
		m_bool = (m_int > 0) ? true : false;
	}

	CConcommand::CConcommand( const char* name, const CommandFunc func ) : m_name( name ), m_pFunc( func )
	{
		assert( name );
		CConcommandExecutor::GetInstance()->AddConcommand( m_name, *this );
	}
	CConcommand::~CConcommand()
	{
	}

	void CConcommand::Execute() const
	{
		if ( m_pFunc != nullptr )
		{
			m_pFunc();
		}
	}

	CConcommandExecutor::CConcommandExecutor( )
	{
	}


	CConcommandExecutor::~CConcommandExecutor( )
	{
	}

	void CConcommandExecutor::AddConcommand( const std::string& cmd, const CConcommand& cmdFuc )
	{
		m_cmdMap.emplace(  cmd, cmdFuc );
	}

	void CConcommandExecutor::AddConvar( const std::string& var, CConvar* cmdVar )
	{
		m_cvarMap.emplace( var, cmdVar );
	}

	void CConcommandExecutor::ExcuteConcommand( )
	{
		if ( m_tokenizer.ArgC() > 0 )
		{
			const std::string& cmdStr = m_tokenizer.ArgV( 0 );
			std::unordered_map<std::string, CConcommand>::iterator findedcmd = m_cmdMap.find( cmdStr );

			if ( findedcmd != m_cmdMap.end( ) )
			{
				findedcmd->second.Execute( );
				return;
			}

			std::unordered_map<std::string, CConvar*>::iterator findedvar = m_cvarMap.find( cmdStr );

			if ( findedvar != m_cvarMap.end( ) )
			{
				if ( m_tokenizer.ArgC() > 1 )
				{
					findedvar->second->SetValue( m_tokenizer.ArgV( 1 ) );
					return;
				}
				else
				{
					std::cout << "[Usage] " << findedvar->second->GetName( )
						<< " Arg" << std::endl;
				}
			}
			else
			{
				std::cout << "Invaild Convar/Concommand" << std::endl;
			}
		}
	}

	void CConcommandExecutor::DoTokenizing( const char* cmd )
	{
		m_tokenizer.DoTokenizing( cmd, ' ' );
	}

	void CConcommandExecutor::PrintCommand( )
	{
		std::cout << "--------------Commmand List---------------" << std::endl;
		for ( std::unordered_map<std::string, CConcommand>::iterator iter = m_cmdMap.begin();
			iter != m_cmdMap.end(); ++iter )
		{
			std::cout << iter->first << std::endl;
		}
	}

	const int CConcommandExecutor::ArgC( ) const
	{
		return m_tokenizer.ArgC();
	}

	const std::string& CConcommandExecutor::ArgV( int index ) const
	{
		return m_tokenizer.ArgV( index );
	}
}

DECLARE_CONCOMMAND( exit )
{
	std::exit( 0 );
}

DECLARE_CONCOMMAND( show_command )
{
	using namespace cmd;
	CConcommandExecutor::GetInstance()->PrintCommand();
}