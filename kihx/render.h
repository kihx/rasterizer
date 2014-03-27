#pragma once

#include "base.h"
#include "matrix.h"

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

		const Data& GetDataConst( size_t index ) const
		{
			assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
			return m_streamSource[index];
		}

		Data& GetData( size_t index )
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


	/* class ConstantBuffer
	*/
	class ConstantBuffer final
	{
	public:
		static const int Capacity = 128;

		enum NamedIndex
		{
			WorldMatrix = 0,
			ViewMatrix = WorldMatrix + 4,
			ProjectionMatrix = ViewMatrix + 4,
			WVPMatrix = ProjectionMatrix + 4
		};

		ConstantBuffer() = default;

		const Vector4& GetVector4( int index )
		{
			assert( index >= 0 && index < Capacity );
			return m_constantBuffer[index];
		}

		const Matrix4& GetMatrix4( int index )
		{
			assert( index >= 0 && index < Capacity );
			// FIXME: is safe such type casting?
			return *( reinterpret_cast< Matrix4* >( &m_constantBuffer[index] ) );
		}

		void SetFloat4( int index, const float* value )
		{
			assert( index >= 0 && index < Capacity );
			m_constantBuffer[index].X = value[0];
			m_constantBuffer[index].Y = value[1];
			m_constantBuffer[index].Z = value[2];
			m_constantBuffer[index].W = value[3];
		}

		void SetVector4( int index, const Vector4& value )
		{
			assert( index >= 0 && index < Capacity );
			m_constantBuffer[index] = value;
		}

		void SetMatrix4( int index, const Matrix4& value )
		{
			SetFloat4( index, value.A[0] );
			SetFloat4( index + 1, value.A[1] );
			SetFloat4( index + 2, value.A[2] );
			SetFloat4( index + 3, value.A[3] );
		}

	private:
		Vector4 m_constantBuffer[Capacity];
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

		ConstantBuffer& GetSharedConstantBuffer()
		{
			return m_sharedConstantBuffer;
		}

	private:
		std::shared_ptr<InputAssembler> m_inputAssembler;
		std::shared_ptr<VertexProcessor> m_vertexProcessor;
		std::shared_ptr<Rasterizer> m_rasterizer;
		std::shared_ptr<PixelProcessor> m_pixelProcessor;
		std::shared_ptr<OutputMerger> m_outputMerger;
		
		std::vector< std::shared_ptr<Texture> > m_renderTargets;

		ConstantBuffer m_sharedConstantBuffer;
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

		const Matrix4& GetWorldMatrix() const
		{
			return m_worldMatrix;
		}
		
		void SetWorldMatrix( const Matrix4& m );

		const Matrix4& GetViewMatrix() const
		{
			return m_viewMatrix;
		}

		void SetViewMatrix( const Matrix4& m );

		const Matrix4& GetProjectionMatrix() const
		{
			return m_projMatrix;
		}

		void SetProjectionMatrix( const Matrix4& m );

	private:
		std::vector<std::shared_ptr<RenderingContext>> m_renderingContexts;
		Matrix4 m_worldMatrix;
		Matrix4 m_viewMatrix;
		Matrix4 m_projMatrix;
	};
};

using kih::RenderingContext;
using kih::RenderingDevice;
