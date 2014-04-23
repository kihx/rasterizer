#pragma once

#include "Comman.h"

#include <map>
#include <string>
#include <iostream>

namespace cmd
{
	typedef void( *CommandFunc )(void);

	class CConcommand
	{
	private:
		std::string m_name;
		CommandFunc m_pFunc;
	public:
		explicit CConcommand( const char* name, const CommandFunc func );
		~CConcommand( );

		void Execute() const;
		const std::string& GetName() const
		{
			return m_name;
		}
	};

	class CConcommandExecutor : public xtozero::CSingletonBase<CConcommandExecutor>
	{
	private:
		std::map<std::string, CConcommand> m_cmdMap;
	public:
		void AddConcommad( const std::string& cmd, const CConcommand& cmdFuc );
		void ExcuteConcommand( const char* cmd ) const;

		CConcommandExecutor( );
		~CConcommandExecutor( );
	};
};

using cmd::CConcommand;

#define DECLARE_CONCOMMAND( name ) \
	void cmdFuc##name(); \
	CConcommand cmd_##name( #name, cmdFuc##name ); \
	void cmdFuc##name( )

