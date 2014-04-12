#ifndef _OUTPUTMERGER_H_
#define _OUTPUTMERGER_H_

#include "pipelineElements.h"

namespace xtozero
{
	class COutputMerger
	{
	private:
		std::vector<unsigned char> m_pDepthBuffer;
		unsigned char* m_pFrameBuffer;
		int m_dpp;

		int m_height;
		int m_width;
	public:
		COutputMerger();
		~COutputMerger();

		void CreateDepthBuffer( int width, int height );
		void ClearDepthBuffer( );
		void SetFrameBuffer( void* pbuffer, int dpp, int width, int height );

		bool ProcessDepthTest( int x, int y, float depth );
		void Process( const std::vector<COmElementDesc>& omInput );
	};
}

#endif