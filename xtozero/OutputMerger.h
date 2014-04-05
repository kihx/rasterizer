#ifndef _OUTPUTMERGER_H_
#define _OUTPUTMERGER_H_

#include "pipelineElements.h"

namespace xtozero
{
	class COutputMerger
	{
	private:
		unsigned char* m_pDepthBuffer;
		unsigned char* m_pFrameBuffer;
		int m_dpp;

		int m_height;
		int m_width;
	public:
		COutputMerger();
		~COutputMerger();

		void CreateDepthBuffer( int width, int height );
		void DestroyDepthBuffer();
		void SetFrameBuffer( void* pbuffer, int dpp, int width, int height );

		bool ProcessDepthTest( int x, int y, float depth );
		void Process( std::vector<COmElementDesc>& omInput );
	};
}

#endif