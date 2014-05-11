#include "CoolD_Command.h"
#include "..\Data\CoolD_Defines.h"
#include "CoolD_ConsoleManager.h"

namespace CoolD
{
//---------------------------------------------------------------
	Command::Command(const string& Name)
		:m_Name(Name)
	{
		GETSINGLE(ConsoleManager).AddCommand(this);
	}
	Command::~Command()
	{

	}

//---------------------------------------------------------------
	VariableCommand::VariableCommand(const string& Name, const string& value)
		: Command(Name)
	{
		SetValue( value );
	}

	VariableCommand::~VariableCommand()
	{
	}

	void VariableCommand::SetValue(const string& Str)
	{
		m_Str = Str;
		m_Integer = stoi(Str);
		m_Float = stof(Str);
	}

	void VariableCommand::SetValue(Dint Integer)
	{
		m_Str = to_string(Integer);
		m_Integer = Integer;
		m_Float = (Dfloat)Integer;
	}

	void VariableCommand::SetValue(Dfloat Float)
	{
		m_Str = to_string(Float);
		m_Integer = (Dint)Float;
		m_Float = Float;
	}

	void VariableCommand::Execute(initializer_list<string> strs)
	{
		if( strs.size() >= 1 )
		{			
			for( auto paramIter = begin(strs); paramIter != end(strs); ++paramIter )
			{	
				if( paramIter == begin(strs) )
				{
					SetValue( *paramIter );	//첫 번째 인자만 취한다. 나머지는 버림					
				}
				else
				{
					cout << (*paramIter) + " is abandoned" << endl;
				}				
			}
		}
		else
		{
			LOG("InitializerList Size < 1");
		}
	}

	void VariableCommand::ShowCommand()
	{
		cout << "\nVariable---------------------------------"<< endl;
		cout << "Name  : " << m_Name << endl;
		cout << "Value : " << m_Str << endl;
		cout << "-----------------------------------------" << endl;
	}

//---------------------------------------------------------------
	FunctionCommand::FunctionCommand(const string& Name, Func_void_initList func /*= nullptr */)
		: Command(Name)
	{
		m_Func = func;
	}

	FunctionCommand::~FunctionCommand()
	{
	}

	void FunctionCommand::Execute(initializer_list<string> strs)
	{
		if( m_Func != nullptr )
		{			
			m_Func(strs);
		}		
	}

	void FunctionCommand::ShowCommand()
	{
		cout << "\nFunction---------------------------------" << endl;
		cout << "Name : " << m_Name << endl;
		cout << "-----------------------------------------" << endl;
	}
};