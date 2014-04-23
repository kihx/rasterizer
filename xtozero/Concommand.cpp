#include "stdafx.h"
#include "Concommand.h"

#include <iostream>

namespace cmd
{
	CConcommand::CConcommand( const char* name, const CommandFunc func ) : m_name( name ), m_pFunc( func )
	{
		assert( name );
		CConcommandExecutor::GetInstance()->AddConcommad( m_name, *this );
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

	void CConcommandExecutor::AddConcommad( const std::string& cmd, const CConcommand& cmdFuc )
	{
		m_cmdMap.insert( std::make_pair( cmd, cmdFuc ) );
	}

	void CConcommandExecutor::ExcuteConcommand( const char* cmd ) const
	{
		std::string cmdStr( cmd );

		std::map<std::string, CConcommand>::const_iterator& iter = m_cmdMap.find( cmdStr );

		if ( iter != m_cmdMap.end( ) )
		{
			iter->second.Execute();
		}
		else
		{
			std::cout << "Invaild Concommand" << std::endl;
		}
	}
}

DECLARE_CONCOMMAND( exit )
{
	std::exit( 0 );
}