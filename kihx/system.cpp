#include "stdafx.h"
#include "system.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


namespace kih
{
	// TODO: dynamically allocate this
	static SystemWindows g_SystemWindows;
	static SYSTEM_INFO g_SystemInfo;

	ISystem* isystem = &g_SystemWindows;


	SystemWindows::SystemWindows()
	{
		isystem = this;

		::ZeroMemory( &g_SystemInfo, sizeof( SYSTEM_INFO ) );
	}

	SystemWindows::~SystemWindows()
	{
		isystem = nullptr;
	}

	void* SystemWindows::ReserveVirtualMemory( int size )
	{
		return ::VirtualAlloc(
			nullptr,
			size,
			MEM_RESERVE,
			PAGE_NOACCESS );
	}

	void* SystemWindows::ReserveAndCommitVirtualMemory( int size )
	{
		return ::VirtualAlloc(
			nullptr,
			size,
			MEM_RESERVE | MEM_COMMIT,
			PAGE_READWRITE );
	}

	bool SystemWindows::CommitVirtualMemory( void* ptr, int size )
	{
		::VirtualAlloc(
			ptr,
			size,
			MEM_COMMIT,
			PAGE_READWRITE );

		return ::GetLastError() == NO_ERROR;
	}

	bool SystemWindows::DecommitVirtualMemory( void* ptr, int size )
	{
		return ::VirtualFree( ptr, size, MEM_DECOMMIT ) == TRUE;
	}

	bool SystemWindows::ReleaseVirtualMemory( void* ptr )
	{
		return ::VirtualFree( ptr, 0, MEM_RELEASE ) == TRUE;
	}

	size_t SystemWindows::GetPageSize() const
	{
		if ( g_SystemInfo.dwPageSize <= 0 )
		{
			::GetSystemInfo( &g_SystemInfo );
		}

		return static_cast<size_t>( g_SystemInfo.dwPageSize );
	}

	size_t SystemWindows::GetAllocationGranularity() const
	{
		if ( g_SystemInfo.dwAllocationGranularity <= 0 )
		{
			::GetSystemInfo( &g_SystemInfo );
		}

		return static_cast<size_t>( g_SystemInfo.dwAllocationGranularity );
	}
}
