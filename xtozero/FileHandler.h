#ifndef _FILEHANDLER_H_
#define _FILEHANDLER_H_

#include <fstream>

class CFileHandler
{
public:
	explicit CFileHandler( const char* pfilepath, std::ios::openmode mode );
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

	std::ifstream& operator>>(unsigned int& buffer)
	{
		m_file >> buffer;
		return m_file;
	}

	std::ifstream& operator>>(long& buffer)
	{
		m_file >> buffer;
		return m_file;
	}

	std::ifstream& operator>>(float& buffer)
	{
		m_file >> buffer;
		return m_file;
	}

	std::ifstream& operator>>(unsigned char& buffer)
	{
		m_file >> buffer;
		return m_file;
	}

	void read( char* pBuffer, std::streamsize size )
	{
		m_file.read( pBuffer, size );
	}

private:
	std::ifstream m_file;
};

#endif

