#ifndef _OUTPUTMERGER_H_
#define _OUTPUTMERGER_H_

#include "pipelineElements.h"
#include "XtzThreadPool.h"

namespace xtozero
{
	class COutputMerger
	{
	private:
		std::vector<int> m_pDepthBuffer;
		unsigned char* m_pFrameBuffer;
		int m_dpp;

		int m_height;
		int m_width;

		SpinLock m_lockObject;

		bool m_isFirst;
	public:
		COutputMerger();
		~COutputMerger();

		void CreateDepthBuffer( const int width, const int height );
		void ClearDepthBuffer( );
		void ClearFrameBuffer( );
		void SetFrameBuffer( void* pbuffer, int dpp, int width, int height );

		bool ProcessDepthTest( const int x, const int y, const float depth );
		void Process( const std::vector<COmElementDesc>& omInput );
		void ProcessParallel( const std::vector<COmElementDesc>& omInput );
	};
}

#endif