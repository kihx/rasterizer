#pragma once

#include "base.h"


namespace kih
{
	/* class ISystem
	*/
	class ISystem
	{
	public:
		// virtual memory
		virtual void* ReserveVirtualMemory( int size ) = 0;
		virtual void* ReserveAndCommitVirtualMemory( int size ) = 0;

		virtual bool CommitVirtualMemory( void* ptr, int size ) = 0;
		virtual bool DecommitVirtualMemory( void* ptr, int size ) = 0;

		virtual bool ReleaseVirtualMemory( void* ptr ) = 0;

		// system information
		virtual size_t GetPageSize() const = 0;
		virtual size_t GetAllocationGranularity() const = 0;
	};

	extern ISystem* isystem;



	/* class SystemWindows
	*/
	class SystemWindows : public ISystem
	{
	public:
		SystemWindows();
		virtual ~SystemWindows();

		// virtual memory
		virtual void* ReserveVirtualMemory( int size );
		virtual void* ReserveAndCommitVirtualMemory( int size );

		virtual bool CommitVirtualMemory( void* ptr, int size );
		virtual bool DecommitVirtualMemory( void* ptr, int size );

		virtual bool ReleaseVirtualMemory( void* ptr );

		// system information
		virtual size_t GetPageSize() const;
		virtual size_t GetAllocationGranularity() const;
	};
}
