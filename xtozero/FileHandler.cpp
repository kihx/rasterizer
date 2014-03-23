#include "stdafx.h"
#include "FileHandler.h"


CFileHandler::CFileHandler( const char* pfilepath )
{
	try
	{
		m_file.open( pfilepath );
	}
	catch ( std::exception e )
	{

	}
}


CFileHandler::~CFileHandler( void )
{
	m_file.close();
}
