#ifndef _FILEHANDLER_H_
#define _FILEHANDLER_H_

#include <fstream>

class CFileHandler
{
public:
	explicit CFileHandler( const char* pfilepath );
	~CFileHandler( void );

	bool is_open()
	{
		return m_file.is_open();
	}

	bool good()
	{
		return m_file.good();
	}

	std::ifstream& operator>>(char* buffer)
	{
		m_file >> buffer;
		return m_file;
	}

	std::ifstream& operator>>(int& buffer)
	{
		m_file >> buffer;
		return m_file;
	}

	std::ifstream& operator>>(float& buffer)
	{
		m_file >> buffer;
		return m_file;
	}

private:
	std::ifstream m_file;
};

#endif

