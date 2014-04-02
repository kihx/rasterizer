#include "stdafx.h"
#include "stage.h"
#include "mesh.h"
#include "texture.h"
#include "vector.h"
#include "matrix.h"
#include "render.h"
#include "random.h"

#include <list>


namespace kih
{
	/* class InputAssembler
	*/
	std::shared_ptr<InputAssemblerOutputStream> InputAssembler::Process( std::shared_ptr<IMesh> inputStream )
	{
		assert( inputStream );

		m_outputStream->Clear();

		// Fill the output stream from the specified mesh.
		if ( inputStream->GetPrimitiveType() == PrimitiveType::Undefined )
		{
			const IrregularMesh* mesh = dynamic_cast< IrregularMesh* >( inputStream.get() );
			if ( mesh == nullptr )
			{
				return m_outputStream;
			}

			// per face rendering
			size_t capacity = mesh->NumVerticesInFace( m_faceIndex );
			m_outputStream->Reserve( capacity );

			// Convert the face geometry to an input stream of the vertex processor.
			size_t face = m_faceIndex;
			size_t numVertices = mesh->NumVerticesInFace( face );
			for ( size_t vert = 0; vert < numVertices; ++vert )
			{
				m_outputStream->Push( mesh->GetVertexInFaceAt( face, vert ) );
			}
		}
		else
		{
			const OptimizedMesh* mesh = dynamic_cast< OptimizedMesh* >( inputStream.get() );
			if ( mesh == nullptr )
			{
				return m_outputStream;
			}

			const auto& vertexBuffer = mesh->GetVertexBufferConst();
			const auto& indexBuffer = mesh->GetIndexBufferConst();

			// per mesh rendering
			size_t numIndices = indexBuffer.Size();
			m_outputStream->Reserve( numIndices );

			// Convert the mesh geometry to an input stream of the vertex processor.
			for ( size_t i = 0; i < numIndices; ++i )
			{
				unsigned short index = indexBuffer.GetIndexConst( i ) - 1;
				m_outputStream->Push( &vertexBuffer.GetVertexConst( index ).x );
			}
		}

		// for wvp transform
		m_outputStream->SetCoordinatesType( inputStream->GetCoordinatesType() );

		return m_outputStream;
	}


	/* class VertexProcessor
	*/
	std::shared_ptr<VertexProcOutputStream> VertexProcessor::Process( std::shared_ptr<VertexProcInputStream> inputStream )
	{
		assert( inputStream );

		m_outputStream->Clear();

		size_t inputSize = inputStream->Size();
		if ( inputSize > 0 )
		{
			// Currently, input and output size is always same.
			m_outputStream->Reserve( inputSize );

			// Trasform vertices considering a coordinates type.
			if ( inputStream->GetCoordinatesType() == CoordinatesType::ReciprocalHomogeneous )
			{
				for ( size_t i = 0; i < inputSize; ++i )
				{
					const auto& vertex = inputStream->GetData( i );
					// No need trasform.
					m_outputStream->Push( vertex.Position );
				}
			}
			else
			{
				// WVP transformation
				const Matrix4& wvp = GetSharedConstantBuffer().GetMatrix4( ConstantBuffer::WVPMatrix );

				for ( size_t i = 0; i < inputSize; ++i )
				{
					const auto& vertex = inputStream->GetData( i );

					Vector4 hPos;
					TransformWVP( vertex.Position, wvp, hPos );

					m_outputStream->Push( hPos );
				}
			}
		}

		m_outputStream->SetCoordinatesType( inputStream->GetCoordinatesType() );

		return m_outputStream;
	}

	void VertexProcessor::TransformWVP( const Vector3& position, const Matrix4& wvp, Vector4& outPosition )
	{
		Vector3_Transform( position, wvp, outPosition );

		//printf( "hpos: %.2f %.2f %.2f\n", hpos.X, hpos.Y, hpos.Z );
	}


	/* class Rasterizer
	*/
	std::shared_ptr<RasterizerOutputStream> Rasterizer::Process( std::shared_ptr<RasterizerInputStream> inputStream )
	{
		assert( inputStream );

		m_outputStream->Clear();

		std::shared_ptr<Texture> rt = GetContext()->GetRenderTaget( 0 );
		if ( rt == nullptr )
		{
			return m_outputStream;
		}

		unsigned short width = static_cast< unsigned short >( rt->Width() );
		unsigned short height = static_cast< unsigned short >( rt->Height() );
		assert( ( width > 0 && height > 0 ) && "invalid operation" );

		// NDC -> Window space
		TransformViewport( inputStream, width, height );

		// rasterization
		DoScanlineConversion( inputStream, m_outputStream, width, height );

		return m_outputStream;
	}

	void Rasterizer::DoScanlineConversion( std::shared_ptr<RasterizerInputStream> inputStream, std::shared_ptr<RasterizerOutputStream> outputStream, unsigned short width, unsigned short height )
	{
		assert( inputStream );
		
		PrimitiveType primitiveType = inputStream->GetPrimitiveType();
		size_t numVerticesPerPrimitive = GetNumberOfVerticesPerPrimitive( primitiveType );

		// Validate the number of vertices considering primitive type.
		size_t numVertices = inputStream->Size();
		if ( numVertices < numVerticesPerPrimitive )
		{
			return;
		}
		else if ( numVertices % numVerticesPerPrimitive != 0 )
		{
			LOG_WARNING( "incomplete primitive" );
			return;
		}

		// FIXME: is this ok??
		m_outputStream->Reserve( width * height );

		// Reserve ET space based on scanlines.
		m_edgeTable.clear();
		m_edgeTable.resize( height );

		// Build an edge table by traversing each primitive in vertices,
		// and draw each primitive.
		size_t numPrimitives = numVertices / numVerticesPerPrimitive;

		// for each primitive
		for ( size_t p = 0; p < numPrimitives; ++p )
		{
			// edge: v0 -> v1
			// v0 = i th vertex
			// v1 = (i + 1) th vertex
			//
			// But if v1 is a vertex of the next primitive,
			// we must hold the first vertex of the current primitive to v1.
			// To keep in simple, we step last -> first -> second -> third ... and last - 1.
			size_t v1Index = p * numVerticesPerPrimitive;
			size_t v0Index = v1Index + ( numVerticesPerPrimitive - 1 );

			// for each vertex
			for ( size_t v = 0; v < numVerticesPerPrimitive; ++v )
			{
				const auto& v0 = inputStream->GetData( v0Index );
				const auto& v1 = inputStream->GetData( v1Index );

				// Make an ET element.
				float xMax = v0.Position.X;
				float xMin = v1.Position.X;
				float zStart = v1.Position.Z;	// Select left's Z.
				float zEnd = v0.Position.Z;
				if ( v0.Position.X < v1.Position.X )
				{
					Swap<float>( xMax, xMin );
					Swap<float>( zStart, zEnd );
				}

				float yMax = v0.Position.Y;
				float yMin = v1.Position.Y;
				if ( v0.Position.Y < v1.Position.Y )
				{
					Swap<float>( yMax, yMin );
				}

				float dx = v0.Position.X - v1.Position.X;
				float dy = v0.Position.Y - v1.Position.Y;
				float slope = ( dy == 0.0f ) ? 0.0f : ( dx / dy );

				// Select a start scanline with Y-axis clipping.
				unsigned short startY = Float_ToInteger<unsigned short>( std::ceil( yMin ) );
				startY = Clamp<unsigned short>( startY, 0, height - 1 );

				// Push this element at the selected scanline.
				m_edgeTable[startY].emplace_back( yMax, xMin, xMax, slope, zStart, zEnd );

				// Update next indices.
				v0Index = v1Index;
				++v1Index;
			}

			GatherPixelsBeingDrawnFromScanlines( outputStream, width, height );
		}
	}

	void Rasterizer::GatherPixelsBeingDrawnFromScanlines( std::shared_ptr<RasterizerOutputStream> outputStream, unsigned short width, unsigned short height )
	{
		assert( outputStream );
		assert( GetContext() );
		assert( height == m_edgeTable.size() && "target height and scanline are mismatched" );

		// FIXME: test code
		// random color for debugging
		byte seedR = static_cast< byte >( Random::Next( 0, 255 ) );
		byte seedG = static_cast< byte >( Random::Next( 0, 255 ) );
		byte seedB = static_cast< byte >( Random::Next( 0, 255 ) );
		Color32 color( seedR, seedG, seedB, 255 );

		// active edge table
		std::list<ActiveEdgeTableElement> aet;

		// for each scanline
		for ( unsigned short y = 0; y < height; ++y )
		{
			// Find outside elements in the AET and remove them.
			auto iter = aet.begin();
			while ( iter != aet.end() )
			{
				const ActiveEdgeTableElement& elem = *iter;
				if ( y >= elem.ET.YMax )
				{
					iter = aet.erase( iter );
				}
				else
				{
					++iter;
				}
			}

			// Push inside ET elements on this scanline into the AET.
			for ( const auto& elem : m_edgeTable[y] )
			{
				if ( y < elem.YMax )
				{
					aet.emplace_back( elem );
				}
			}

			if ( aet.empty() )
			{
				continue;
			}

			// Sort AET elements from left to right.	
			if ( aet.size() > 1 )
			{
				aet.sort();
			}

			// Gather pixels being drawn using the AET.
			for ( auto iter = aet.begin(); iter != aet.end(); ++iter )
			{
				// First, select left (=current) and right (=next) edge elements.
				ActiveEdgeTableElement& elemLeft = *iter;
				if ( ++iter == aet.end() )
				{
					elemLeft.CurrentX += elemLeft.ET.Slope;
					break;
				}
				ActiveEdgeTableElement& elemRight = *iter;

				// Approximate intersection pixels betweeen edges on the scanline.
				if ( elemLeft.CurrentX != elemRight.CurrentX )
				{
					unsigned short xLeft = Float_ToInteger<unsigned short>( std::round( elemLeft.CurrentX ) );
					unsigned short xRight = Float_ToInteger<unsigned short>( std::round( elemRight.CurrentX ) );

					// clipping on RT
					xLeft = Max<unsigned short>( xLeft, 0 );
					xRight = Min<unsigned short>( xRight, width );

					if ( xLeft != xRight )
					{
						// Compute lerp ratio for the left-end pixel and the right-end pixel.
						float lerpRatioLeft = ( elemLeft.CurrentX - elemLeft.ET.XMin ) / max( elemLeft.ET.XMax - elemLeft.ET.XMin, FLT_EPSILON );
						float lerpRatioRight = ( elemRight.CurrentX - elemRight.ET.XMin ) / max( elemRight.ET.XMax - elemRight.ET.XMin, FLT_EPSILON );

						// Lerp depth values of L and R between their own edges.
						float depthLeft = Lerp( elemLeft.ET.ZStart, elemLeft.ET.ZEnd, lerpRatioLeft );
						float depthRight = Lerp( elemRight.ET.ZStart, elemRight.ET.ZEnd, lerpRatioRight );

						// TODO: other lerp vars...

						// an incremetal approach to lerp depth
						float depthRatioFactor = 1.0f / ( xRight - xLeft );
						float ddxDepth = ( depthRight - depthLeft ) * depthRatioFactor;
						float interpolatedDepth = depthLeft;

						// Push inside pixels between intersections into the output stream.
						for ( unsigned short x = xLeft; x < xRight; ++x )
						{
#if 0	// directly compute lerp depth (not used)
							// Lerp a depth value at the point.
							float interpolatedDepth = Lerp( depthLeft, depthRight, ( x - xLeft ) * depthRatioFactor );
#endif
							outputStream->Push( x, y, interpolatedDepth, color );

							// Update the next depth value incrementally.
							interpolatedDepth += ddxDepth;
						}
					}
				}

				// Update the next position incrementally.
				elemLeft.CurrentX += elemLeft.ET.Slope;
				elemRight.CurrentX += elemRight.ET.Slope;
			}
		}

		for ( unsigned short y = 0; y < height; ++y )
		{
			m_edgeTable[y].clear();
		}
	}

	void Rasterizer::TransformViewport( std::shared_ptr<RasterizerInputStream> inputStream, unsigned short width, unsigned short height )
	{
		// viewport transform for projective coordinates
		if ( inputStream->GetCoordinatesType() != CoordinatesType::Projective )
		{
			return;
		}

		float factorX = width * 0.5f;
		float factorY = height * 0.5f;

		size_t numVertices = inputStream->Size();
		for ( size_t v = 0; v < numVertices; ++v )
		{
			auto& data = inputStream->GetData( v );

			// perspective division
			data.Position /= data.Position.W;

			// NDC -> viewport
			data.Position.X *= factorX;
			data.Position.X += factorX;	// + x

			data.Position.Y *= factorY;
			data.Position.Y += factorY;	// + y

			// [-1, 1] to [0, 1]
			data.Position.Z *= 0.5f;
			data.Position.Z += 0.5f;

			//printf( "data: %.2f %.2f %.2f\n", data.Position.X, data.Position.Y, data.Position.Z );
			//if ( (v + 1) % 3 == 0 )
			//{
			//	printf( "\n" );
			//}
		}
	}


	/* class PixelProcessor::DepthBufferingParamPack
	*/
	PixelProcessor::DepthBufferingParamPack::DepthBufferingParamPack( std::shared_ptr<Texture> ds ) :
		m_ds( ds ),
		m_ptr( nullptr )
	{
		if ( ds == nullptr )
		{
			// This is NOT an error. We accept this case.
			return;
		}

		m_ds = ds;

		// TODO: implementation for a floating point depth buffer
		// Currently, we assume that the size of a depth buffer is one byte.
		assert( ( ds->Format() == ColorFormat::D8S24 ) && "floating point depth buffer is not implemented yet" );

		// Get raw memory from the depth stencil.
		if ( !m_ds->Lock( reinterpret_cast< void** >( &m_ptr ) ) )
		{
			return;
		}

		m_width = m_ds->Width();
		m_stride = GetBytesPerPixel( m_ds->Format() );
	}

	PixelProcessor::DepthBufferingParamPack::~DepthBufferingParamPack()
	{
		if ( m_ds )
		{
			m_ds->Unlock();
		}
	}


	/* class PixelProcessor
	*/
	std::shared_ptr<PixelProcOutputStream> PixelProcessor::Process( std::shared_ptr<PixelProcInputStream> inputStream )
	{
		assert( inputStream );

		m_outputStream->Clear();

		size_t inputStreamSize = inputStream->Size();
		if ( inputStreamSize <= 0 )
		{
			return m_outputStream;
		}

		//assert( ( GetContext()->GetRenderTaget( 1 ) == nullptr ) && "MRT is not implemented yet" );

		// UNDONE: Currently, we assume RTs is only one.
		std::shared_ptr<Texture> rt = GetContext()->GetRenderTaget( 0 );
		if ( rt == nullptr )
		{
			return m_outputStream;
		}

		// Get raw memory from the render target.
		LockGuardPtr<Texture> guardRT( rt );
		byte* bufferRT = static_cast< byte* >( guardRT.Ptr() );
		if ( bufferRT == nullptr )
		{
			return m_outputStream;
		}

		int widthRT = rt->Width();
		int strideRT = GetBytesPerPixel( rt->Format() );


		// Make a parameter pack for depth writing.
		std::shared_ptr<Texture> ds = GetContext()->GetDepthStencil();
		DepthBufferingParamPack depthBufferingParam( ds );

		// Load pixel shader constants.
		//const Vector4& diffuseColor = GetSharedConstantBuffer().GetVector4( ConstantBuffer::DiffuseColor );
		//Color32 color = Vector4_ToColor32( color );


		// Now, do per-pixel operations here.
		for ( size_t i = 0; i < inputStreamSize; ++i )
		{
			const auto& fragment = inputStream->GetData( i );

			// depth buffering
			if ( depthBufferingParam.IsValid() )
			{
				if ( !DoDepthBuffering( depthBufferingParam, fragment ) )
				{
					continue;
				}
			}

			Color32 color = fragment.Color;

#if 0	// depth visualization
			byte d = Float_ToByte( fragment.Depth );
			color.R = color.G = color.B = d;
#endif

			// TODO: pixel shading

#if 1	// faster code
			byte* base = bufferRT + ( ( ( widthRT * fragment.PY ) + fragment.PX ) * strideRT );
			*( base ) = color.R;
			*( base + 1 ) = color.G;
			*( base + 2 ) = color.B;
#else
			//rt->WriteTexel( fragment.PX, fragment.PY, fragment.Color.Value );
#endif
		}

		return m_outputStream;
	}

	bool PixelProcessor::DoDepthBuffering( DepthBufferingParamPack& param, const PixelProcData& fragment )
	{
		assert( param.IsValid() );
		assert( GetContext() );

		byte* addr = param.GetAddress( fragment );
		if ( addr == nullptr )
		{
			LOG_WARNING( "invalid operation" );
			return true;	// This is correct because no depth func means that a depth test is always passed.
		}

		byte& dst = *addr;
		byte src = Float_ToByte( fragment.Depth );

#if 0
		// inline lamda with [=] capture block
		using DepthTestFunc = std::function< bool() >;
		DepthTestFunc lessEqual = [=]() { return value <= ref; };
#endif

		// Call a functional object for depth test.
		if ( !GetContext()->CallDepthFunc( src, dst ) )
		{
			return false;
		}

		// depth write
		// FIXME: perform this in an output merger
		if ( GetContext()->DepthWritable() )
		{
			dst = src;
		}

		return true;
	}


	/* class OutputMerger
	*/
	std::shared_ptr<OutputMergerOutputStream> OutputMerger::Process( std::shared_ptr<OutputMergerInputStream> inputStream )
	{
		//OutputMergerInputStream* pOut = new OutputMergerInputStream();

		//inputStream.reset();
		//return std::shared_ptr<OutputMergerInputStream>( pOut );
		return inputStream;
	}
};
