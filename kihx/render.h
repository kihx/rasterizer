#pragma once

#include "base.h"

#include <memory>
#include <vector>


namespace kih
{
	class Mesh;

	class ConstantBuffer;
	class Texture;

	class VertexProcInputStream;	
	typedef VertexProcInputStream InputAssemblerOutputStream;

	class RasterizerInputStream;
	typedef RasterizerInputStream VertexProcOutputStream;

	class PixelProcInputStream;
	typedef PixelProcInputStream RasterizerOutputStream;

	class OutputMergerInputStream;
	typedef OutputMergerInputStream PixelProcOutputStream;
	typedef OutputMergerInputStream OutputMergerOutputStream;

	class InputAssembler;
	class VertexProcessor;
	class Rasterizer;
	class PixelProcessor;
	class OutputMerger;
	class RenderingContext;


	/* class IRenderingProcessor
	*/
	template<class InputStream, class OutputStream>
	class IRenderingProcessor
	{
	public:
		virtual std::shared_ptr<OutputStream> Process( std::shared_ptr<InputStream> inputStream ) = 0;
	};


	/* class BaseInputOutputStream
	*/
	template<class Data>
	class BaseInputOutputStream
	{
		NONCOPYABLE_CLASS( BaseInputOutputStream );

	public:
		BaseInputOutputStream() = default;
		virtual ~BaseInputOutputStream() = default;

		template <typename... Args>
		void Push( Args&&... args )
		{
			m_streamSource.emplace_back( std::forward<Args>( args )... );
		}

		const Data* GetStreamSource() const
		{
			if ( m_streamSource.empty() )
			{
				return nullptr;
			}
			else
			{
				return &m_streamSource[0];
			}
		}

		const Data& GetData( size_t index ) const
		{
			assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
			return m_streamSource[index];
		}

		size_t Size() const
		{
			return m_streamSource.size();
		}

		void Reserve( size_t capacity )
		{
			m_streamSource.reserve( capacity );
		}

	private:
		std::vector<Data> m_streamSource;
	};

	

	/* class RenderingContext
	*/
	class RenderingContext final
	{
		NONCOPYABLE_CLASS( RenderingContext )

	public:
		explicit RenderingContext( size_t numRenderTargets );

		void Clear( byte r, byte g, byte b, byte a );

		void Draw( std::shared_ptr<Mesh> mesh );

		std::shared_ptr<Texture> GetRenderTaget( size_t index )
		{
			assert( ( index >= 0 && index < m_renderTargets.size() ) && "out of ranged index" );
			return m_renderTargets[index];
		}

		bool SetRenderTarget( std::shared_ptr<Texture> texture, size_t index )
		{
			assert( ( index >= 0 && index < m_renderTargets.size() ) && "out of ranged index" );
			m_renderTargets[index] = texture;
			return true;
		}

	private:
		std::shared_ptr<InputAssembler> m_inputAssembler;
		std::shared_ptr<VertexProcessor> m_vertexProcessor;
		std::shared_ptr<Rasterizer> m_rasterizer;
		std::shared_ptr<PixelProcessor> m_pixelProcessor;
		std::shared_ptr<OutputMerger> m_outputMerger;
		std::vector< std::shared_ptr<Texture> > m_renderTargets;
	};


	/* class RenderingDevice
	*/
	class RenderingDevice final : public Singleton<RenderingDevice>
	{
		friend class Singleton<RenderingDevice>;

	private:
		RenderingDevice()
		{
		};

	public:
		~RenderingDevice()
		{
		};

		std::shared_ptr<RenderingContext> CreateRenderingContext();

	private:
		std::vector<std::shared_ptr<RenderingContext>> m_renderingContexts;
	};
};

using kih::RenderingContext;
using kih::RenderingDevice;
