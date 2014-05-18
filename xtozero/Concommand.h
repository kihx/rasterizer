#pragma once

#include "Comman.h"

#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>

namespace cmd
{
	typedef void( *CommandFunc )(void);

	class CTokenizer
	{
	private:
		std::vector<std::string> m_argv;
	public:
		CTokenizer( ) {}
		~CTokenizer( ) {}

		const int ArgC() const;
		const std::string& ArgV( int index ) const;

		void DoTokenizing( const char* str, const char token );
	};

	class CConvar
	{
	private:
		std::string m_name;
		std::string m_value;
		float m_float;
		int m_int;
		bool m_bool;
	public:
		explicit CConvar( const char* name , const char* value );
		const std::string& GetName() const;

		const int GetInt() const;
		const float GetFloat( ) const;
		const bool GetBool() const;
		const char* GetChar() const;
		void SetValue( const char* value );
		void SetValue( const std::string& value );
	};

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
		std::unordered_map<std::string, CConcommand>	m_cmdMap;
		std::unordered_map<std::string, CConvar*>		m_cvarMap;
		CTokenizer							m_tokenizer;
	public:
		void AddConcommand( const std::string& cmd, const CConcommand& cmdFuc );
		void AddConvar( const std::string& var, CConvar* cmdVar );
		void ExcuteConcommand( );
		void DoTokenizing( const char* cmd );
		void PrintCommand( );

		CConcommandExecutor( );
		~CConcommandExecutor( );
		
		const int ArgC( ) const;
		const std::string& ArgV( int index ) const;
	};
};

using cmd::CConcommand;

#define DECLARE_CONCOMMAND( name ) \
	void cmdFuc##name(); \
	CConcommand cmd_##name( #name, cmdFuc##name ); \
	void cmdFuc##name( )

