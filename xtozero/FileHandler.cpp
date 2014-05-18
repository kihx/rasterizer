#include "stdafx.h"
#include "FileHandler.h"


CFileHandler::CFileHandler( const char* pfilepath, std::ios::openmode mode )
{
	try
	{
		m_file.open( pfilepath, mode );
	}
	catch ( std::exception e )
	{

	}
}


CFileHandler::~CFileHandler( void )
{
	m_file.close();
}
