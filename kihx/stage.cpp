#include "stdafx.h"
#include "stage.h"
#include "mesh.h"
#include "texture.h"
#include "vector.h"
#include "matrix.h"
#include "render.h"
#include "threading.h"
#include "random.h"
#include "concommand.h"

#include <list>


//#define EARLY_Z_CULLING

//#define BYPASS_PIXEL_SHADING


namespace kih
{	
	/* class DepthBuffering
	*/
	DepthBuffering::DepthBuffering( RenderingContext* context ) :
		m_context( context ),
		m_ds( nullptr ),
		m_ptr( nullptr ),
		m_format( ColorFormat::Unknown ),
		m_width( 0 ),
		m_stride( 0 ),
		m_depthFunc( DepthFunc::None )
	{
		if ( m_context == nullptr )
		{
			// This is NOT an error. We accept this case.
			return;
		}

		m_ds = context->GetDepthStencil();
		if ( m_ds == nullptr )
		{
			// This is NOT an error. We accept this case.
			return;
		}
		
		// Get raw memory from the depth stencil.
		if ( !m_ds->Lock( reinterpret_cast< void** >( &m_ptr ) ) )
		{
			return;
		}

		m_format = m_ds->Format();
		m_width = m_ds->Width();
		m_stride = GetBytesPerPixel( m_format );
		m_depthFunc = context->DepthFunction();
	}

	DepthBuffering::~DepthBuffering()
	{
		if ( m_ds )
		{
			m_ds->Unlock();
		}
	}

	template<typename T>
	bool DepthBuffering::ExecuteInternal( unsigned short x, unsigned short y, T depth )
	{
		VerifyReentry( 1 );
		
		Assert( IsValid() );
		Assert( x >= 0 && x < m_width );
		Assert( y >= 0 && y < m_ds->Height() );

		byte* addr = GetAddress( x, y );
		if ( addr == nullptr )
		{
			throw std::range_error( "out of ranged x or y coordinates to access the depth-stencil buffer" );
		}

		T& dst = *( reinterpret_cast<T*>( addr ) );

#ifdef DEPTHFUNC_LAMDA
		// Call a functional object for depth test.
		if ( !m_context->CallDepthFunc( depth, dst ) )
		{
			return false;
		}
#else
		switch ( m_depthFunc )
		{
		case DepthFunc::Not:
			if ( !( depth != dst ) )
			{
				return false;
			}
			break;

		case DepthFunc::Equal:
			if ( !( depth == dst ) )
			{
				return false;
			}
			break;

		case DepthFunc::Less:
			if ( !( depth < dst ) )
			{
				return false;
			}
			break;

		case DepthFunc::LessEqual:
			if ( !( depth <= dst ) )
			{
				return false;
			}
			break;

		case DepthFunc::Greater:
			if ( !( depth > dst ) )
			{
				return false;
			}
			break;

		case DepthFunc::GreaterEqual:
			if ( !( depth >= dst ) )
			{
				return false;
			}
			break;

		default:
			break;
		}
#endif

		// depth write
		if ( m_context->DepthWritable() )
		{
			dst = depth;
		}

		return true;
	}


	/* class InputAssembler
	*/
	std::shared_ptr<InputAssemblerOutputStream> InputAssembler::Process( std::shared_ptr<IMesh> inputStream )
	{
		Assert( inputStream );

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
		Assert( inputStream );

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

	void VertexProcessor::TransformWVP( const Vector3& position, const Matrix4& wvp, Vector4& outPosition ) const
	{
		Vector3_Transform( position, wvp, outPosition );

		//printf( "hpos: %.2f %.2f %.2f\n", hpos.X, hpos.Y, hpos.Z );
	}


	/* class Rasterizer
	*/
	std::shared_ptr<RasterizerOutputStream> Rasterizer::Process( std::shared_ptr<RasterizerInputStream> inputStream )
	{
		Assert( inputStream );

		m_outputStream->Clear();

		const std::shared_ptr<Texture>& rt = GetContext()->GetRenderTagetConst( 0 );
		if ( rt == nullptr )
		{
			return m_outputStream;
		}

		unsigned short width = static_cast< unsigned short >( rt->Width() );
		unsigned short height = static_cast< unsigned short >( rt->Height() );
		Assert( ( width > 0 && height > 0 ) && "invalid operation" );

		// NDC -> Window space
		TransformViewport( inputStream, width, height );

		// rasterization
		DoScanlineConversion( inputStream, m_outputStream, width, height );

		return m_outputStream;
	}

	void Rasterizer::DoScanlineConversion( std::shared_ptr<RasterizerInputStream> inputStream, std::shared_ptr<RasterizerOutputStream> outputStream, unsigned short width, unsigned short height )
	{
		Assert( inputStream );
		
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
				
#ifdef EARLY_Z_CULLING
		DepthBuffering depthBuffering( GetContext() );
#else
		DepthBuffering depthBuffering( nullptr );
#endif

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
					Swap( xMax, xMin );
					Swap( zStart, zEnd );
				}

				float yMax = v0.Position.Y;
				float yMin = v1.Position.Y;
				if ( v0.Position.Y < v1.Position.Y )
				{
					Swap( yMax, yMin );
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

			GatherPixelsBeingDrawnFromScanlines( outputStream, width, height, depthBuffering );
		}
	}

	void Rasterizer::GatherPixelsBeingDrawnFromScanlines( std::shared_ptr<RasterizerOutputStream> outputStream, unsigned short width, unsigned short height, DepthBuffering& depthBuffering )
	{
		Assert( outputStream );
		Assert( GetContext() );
		Assert( height == m_edgeTable.size() && "target height and scanline are mismatched" );

		// FIXME: test code
		Color32 color( 255, 255, 255, 255 );

		// active edge table
		std::list<ActiveEdgeTableElement> aet;

		// for each scanline
		for ( unsigned short y = 0; y < height; ++y )
		{
			if ( !UpdateActiveEdgeTable( aet, y ) )
			{
				continue;
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

#ifdef EARLY_Z_CULLING
							// depth buffering
							if ( depthBuffering.IsValid() )
							{
								if ( !depthBuffering.Execute( x, y, interpolatedDepth ) )
								{
									continue;
								}
							}
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

	bool Rasterizer::UpdateActiveEdgeTable( std::list<ActiveEdgeTableElement> &aet, unsigned short scanline ) const
	{
		Assert( scanline >= 0 && scanline < m_edgeTable.size() );

		// Find outside elements in the AET and remove them.
		auto iter = aet.begin();
		while ( iter != aet.end() )
		{
			const ActiveEdgeTableElement& elem = *iter;
			if ( scanline >= elem.ET.YMax )
			{
				iter = aet.erase( iter );
			}
			else
			{
				++iter;
			}
		}

		// Push inside ET elements on this scanline into the AET.
		for ( const auto& elem : m_edgeTable[scanline] )
		{
			if ( scanline < elem.YMax )
			{
				aet.emplace_back( elem );
			}
		}

		// Pass if AET is empty.
		if ( aet.empty() )
		{
			return false;
		}

		// Sort AET elements from left to right.	
		if ( aet.size() > 1 )
		{
			// std::list::sort() is very very slow. :(
			// So, if the size of AET is only two,
			// we reverse AET where node0 > node1 instead of sort().
			// This optimization is meaningful because a triangle mostly haves two edges on a scanline.
			if ( aet.size() == 2 )
			{
				const auto& node0 = aet.front();
				const auto& node1 = aet.back();
				if ( node0.CurrentX > node1.CurrentX )
				{
					aet.reverse();
				}
			}
			else
			{
				aet.sort();
			}
		}

		return true;
	}

	void Rasterizer::TransformViewport( std::shared_ptr<RasterizerInputStream> inputStream, unsigned short width, unsigned short height ) const
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

			data.Position.Y *= -factorY;
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
		

	/* class PixelProcessor
	*/
	std::shared_ptr<PixelProcOutputStream> PixelProcessor::Process( std::shared_ptr<PixelProcInputStream> inputStream )
	{
		Assert( inputStream );
		
		m_outputStream->Clear();

		size_t inputStreamSize = inputStream->Size();
		if ( inputStreamSize <= 0 )
		{
			return m_outputStream;
		}

#ifdef BYPASS_PIXEL_SHADING
		// do nothing
#else
		// Load pixel shader constants.
		const Vector4& diffuseColor = GetSharedConstantBuffer().GetVector4( ConstantBuffer::DiffuseColor );
		Color32 color = Vector4_ToColor32( diffuseColor );


		// Now, do per-pixel operations here.
		for ( size_t i = 0; i < inputStreamSize; ++i )
		{
			auto& fragment = inputStream->GetData( i );

			//Color32 color = fragment.Color;			

			// TODO: pixel shading
			//

			// update
			fragment.Color = color;
		}
#endif

		// Move stream memory from input to output for performance.
		m_outputStream->MoveFrom( std::move( *inputStream.get() ) );

		return m_outputStream;
	}


	/* class OutputMerger
	*/
	std::shared_ptr<OutputMergerOutputStream> OutputMerger::Process( std::shared_ptr<OutputMergerInputStream> inputStream )
	{
		Assert( inputStream );

		VerifyReentry( 1 );

		size_t inputStreamSize = inputStream->Size();
		if ( inputStreamSize <= 0 )
		{
			return m_outputStream;
		}

		// UNDONE: Currently, we assume RTs is only one.
		Assert( ( GetContext()->NumberOfRenderTargets() == 1 ) && "MRT is not implemented yet" );
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

#ifdef EARLY_Z_CULLING
		// do nothing
#else
		DepthBuffering depthBuffering( GetContext() );
#endif

		// Now, write color on render targets and the depth stencil buffer here.
		for ( size_t i = 0; i < inputStreamSize; ++i )
		{
			const auto& fragment = inputStream->GetData( i );

#ifdef EARLY_Z_CULLING
			// do nothing
#else
			// depth buffering
			if ( depthBuffering.IsValid() )
			{
				if ( !depthBuffering.Execute( fragment.PX, fragment.PY, fragment.Depth ) )
				{
					continue;
				}
			}
#endif

			Color32 color = fragment.Color;

#if 0	// depth visualization
			byte d = Float_ToByte( fragment.Depth );
			color.R = color.G = color.B = d;
#endif

			// TODO: blending operation

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
};
