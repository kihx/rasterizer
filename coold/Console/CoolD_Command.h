#pragma once
#include "..\Data\CoolD_Defines.h"
#include "..\Data\CoolD_Type.h"

namespace CoolD
{
	class Command
	{
	private:
		Command() = delete;
		Command(const Command&) = delete;
		Command& operator=(const Command&) = delete;
	protected:
		Command(const string& Name);

	public:
		virtual ~Command();

	public:
		inline string GetCommandName()		const	{ return m_Name; }

	public:
		virtual void Execute(initializer_list<string> strs) = 0;
		virtual void ShowCommand() = 0;

	protected:
		const string m_Name;
	};

	class VariableCommand : public Command
	{
	public:
		VariableCommand(const string& Name, const string& value);
		virtual ~VariableCommand();

	public:
		inline const string& String()	const { return m_Str; }
		inline const Dint Integer()		const { return m_Integer; }
		inline const Dfloat Float()		const { return m_Float; }
		inline const Dbool Bool()		const { return m_Float != 0; }

	public:
		void SetValue(const string& Str);
		void SetValue(Dint Integer);
		void SetValue(Dfloat Float);

	public:
		virtual void Execute(initializer_list<string> strs);
		virtual void ShowCommand();

	private:
		string	m_Str;
		Dint	m_Integer;
		Dfloat	m_Float;
	};

	
	class FunctionCommand : public Command
	{			
	public:
		FunctionCommand(const string& Name, Func_void_initList func = nullptr);
		virtual ~FunctionCommand();
	public:
		virtual void Execute(initializer_list<string> strs);
		virtual void ShowCommand();

	private:
		Func_void_initList m_Func;
	};
};