#include "stdafx.h"
#include "concommand.h"
#include "matrix.h"
#include "mesh.h"
#include "random.h"
#include "render.h"
#include "stage.h"
#include "texture.h"
#include "threading.h"
#include "vector.h"


#define USE_SSE_OPTIMZATION

//#define EARLY_Z_CULLING

//#define BYPASS_PIXEL_SHADING


namespace kih
{
	/* class DepthBuffering
	*/
	DepthBuffering::DepthBuffering( RenderingContext* context ) :
		m_ds( nullptr ),
		m_ptr( nullptr ),
		m_format( ColorFormat::Unknown ),
		m_width( 0 ),
		m_stride( 0 ),
		m_depthFunc( DepthFunc::None ),
		m_depthWritable( false )
	{
		if ( context == nullptr )
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
		m_depthWritable = context->DepthWritable();
	}

	DepthBuffering::~DepthBuffering()
	{
		if ( m_ds )
		{
			m_ds->Unlock();
		}
	}


	/* class InputAssembler
	*/
	std::shared_ptr<InputAssemblerOutputStream> InputAssembler::Process( const std::shared_ptr<IMesh>& inputStream )
	{
		Assert( inputStream );
		Assert( GetContext() );

		m_outputStream->Clear();

		// Fill the output stream from the specified mesh.
		if ( inputStream->GetPrimitiveType() == PrimitiveType::Undefined )
		{
			const IrregularMesh* mesh = dynamic_cast< const IrregularMesh* >( inputStream.get() );
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
			if ( GetContext()->IsFixedPipelineMode() )
			{
				m_outputFixedPipeline->Clear();

				for ( size_t vert = 0; vert < numVertices; ++vert )
				{
					m_outputFixedPipeline->Push( Vector3( mesh->GetVertexInFaceAt( face, vert ) ) );
				}
			}
			else
			{
				for ( size_t vert = 0; vert < numVertices; ++vert )
				{
					m_outputStream->Push( mesh->GetVertexInFaceAt( face, vert ) );
				}
			}
		}
		else
		{
			const OptimizedMesh* mesh = dynamic_cast< const OptimizedMesh* >( inputStream.get() );
			if ( mesh == nullptr )
			{
				return m_outputStream;
			}

			const auto& vertexBuffer = mesh->GetVertexBufferConst();
			const auto& indexBuffer = mesh->GetIndexBufferConst();

			// per mesh rendering
			size_t numIndices = indexBuffer.Size();
			m_outputStream->Reserve( numIndices );

			if ( GetContext()->IsFixedPipelineMode() )
			{
				m_outputFixedPipeline->Clear();

				// In fixed pipeline mode, transform vertices onto WVP.
				const Matrix4& wvp = GetSharedConstantBuffer().GetMatrix4( ConstantBuffer::WVPMatrix );
				Vector4 hPos;

				for ( size_t i = 0; i < numIndices; ++i )
				{
					unsigned short index = indexBuffer.GetIndexConst( i ) - 1;
					const auto& vertex = vertexBuffer.GetVertexConst( index );

					Vector3 v( vertex.x, vertex.y, vertex.z );
#ifdef USE_SSE_OPTIMZATION
					Vector3_TransformSSE( v, wvp, hPos );
#else
					Vector3_Transform( v, wvp, hPos );
#endif
					m_outputFixedPipeline->Push( hPos );
				}
			}
			else
			{
				// Convert the mesh geometry to an input stream of the vertex processor.
				for ( size_t i = 0; i < numIndices; ++i )
				{
					unsigned short index = indexBuffer.GetIndexConst( i ) - 1;
					m_outputStream->Push( &vertexBuffer.GetVertexConst( index ).x );
				}
			}
		}

		// for wvp transform
		m_outputStream->SetCoordinatesType( inputStream->GetCoordinatesType() );

		return m_outputStream;
	}


	/* class VertexShader
	*/
	std::shared_ptr<VertexShaderOutputStream> VertexShader::Process( const std::shared_ptr<VertexShaderInputStream>& inputStream )
	{
		Assert( inputStream );
		Assert( GetContext() );

		if ( GetContext()->IsFixedPipelineMode() )
		{
			return m_outputStream;
		}


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
				Vector4 hPos;

				for ( size_t i = 0; i < inputSize; ++i )
				{
					const auto& vertex = inputStream->GetData( i );

					TransformWVP( vertex.Position, wvp, hPos );

					m_outputStream->Push( hPos );
				}
			}
		}

		m_outputStream->SetCoordinatesType( inputStream->GetCoordinatesType() );

		return m_outputStream;
	}

	void VertexShader::TransformWVP( const Vector3& position, const Matrix4& wvp, Vector4& outPosition ) const
	{
#ifdef USE_SSE_OPTIMZATION
		Vector3_TransformSSE( position, wvp, outPosition );
#else
		Vector3_Transform( position, wvp, outPosition );
#endif
	}


	/* class Rasterizer
	*/
	std::shared_ptr<RasterizerOutputStream> Rasterizer::Process( const std::shared_ptr<RasterizerInputStream>& inputStream )
	{
		Assert( inputStream );

		m_outputStream->Clear();

		const Viewport& viewport = GetContext()->GetViewport();
		unsigned short width = static_cast< unsigned short >( viewport.Width );
		unsigned short height = static_cast< unsigned short >( viewport.Height );
		Assert( ( width > 0 && height > 0 ) && "invalid operation" );

		// NDC -> Window space
		TransformViewport( inputStream, width, height );

		// rasterization
		DoScanlineConversion( inputStream, width, height );

		return m_outputStream;
	}

	void Rasterizer::DoScanlineConversion( const std::shared_ptr<RasterizerInputStream>& inputStream, unsigned short width, unsigned short height )
	{
		Assert( inputStream );
		Assert( m_outputStream );
		
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
				
		// Reserve predictible sized memory.
		m_outputStream->Reserve( width * height );

		// Rasterize primitives
		if ( numVerticesPerPrimitive == 3 )
		{
#ifdef USE_SSE_OPTIMZATION
			RasterizeUsingBarycentricCoordinatesSSE( inputStream, width, height );
#else
			RasterizeUsingBarycentricCoordinates( inputStream, width, height );
#endif
		}
		else
		{
			RasterizeUsingEdgeTable( inputStream, numVerticesPerPrimitive, width, height );
		}
	}

	void Rasterizer::RasterizeUsingEdgeTable( const std::shared_ptr<RasterizerInputStream>& inputStream, size_t numVerticesPerPrimitive, unsigned short width, unsigned short height )
	{
		Assert( inputStream );

#ifdef EARLY_Z_CULLING
		DepthBuffering depthBuffering( GetContext() );
		depthBuffering.SetWritable( false );	// depth-test only on the rasterizer
#else
		DepthBuffering depthBuffering( nullptr );
#endif

		// Reserve ET space based on scanlines.
		m_edgeTable.clear();
		m_edgeTable.resize( height );

		// the list of indices of edges in ET to reduce traversing empty elements.
		StlVector<unsigned short> validEdgeIndices;
		validEdgeIndices.reserve( numVerticesPerPrimitive );

		// active edge table
		StlVector<ActiveEdgeTableElement> aet;
		aet.reserve( 8 );

		// Build an edge table by traversing each primitive in vertices,
		// and draw each primitive.
		size_t numVertices = inputStream->Size();
		size_t numPrimitives = numVertices / numVerticesPerPrimitive;

		// for each primitive
		for ( size_t p = 0; p < numPrimitives; ++p )
		{
			// This enables code to skip uncovered scanelines of the primitive.
			// We only interest scanlines from min to max.
			unsigned short startY = 0;
			unsigned short minScanline = 0xFFFF;
			unsigned short endY = 0;
			unsigned short maxScanline = 0;
			unsigned short maxY = height - 1;

			bool isCulled = false;

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

				// backface culling
				if ( v == 0 && m_cullMode != CullMode::None )
				{
					const auto& v2 = inputStream->GetData( v1Index + 1 );

					// FIXME: CW/CCW
#ifdef USE_SSE_OPTIMZATION
					if ( SSE::IsBackFace( v0.Position, v1.Position, v2.Position ) )
#else
					if ( IsBackFace( v0.Position, v1.Position, v2.Position ) )
#endif
					{
						isCulled = true;
						break;
					}
				}

				// Make an ET element.
				float xMax = Max( v0.Position.X, v1.Position.X );
				float xMin = Min( v0.Position.X, v1.Position.X );
				float zStart = v1.Position.Z;	// Select xMin's Z.
				float zEnd = v0.Position.Z;
				if ( v0.Position.X < v1.Position.X )
				{
					Swap( zStart, zEnd );
				}

				float yMax = Max( v0.Position.Y, v1.Position.Y );
				float yMin = Min( v0.Position.Y, v1.Position.Y );

				// Select start and end scanlines with Y-axis clipping.
#ifdef USE_SSE_OPTIMZATION
				startY = static_cast< unsigned short >( SSE::Ceil( yMin ) );
#else
				startY = static_cast< unsigned short >( std::ceil( yMin ) );
#endif
				startY = Clamp<unsigned short>( startY, 0, maxY );
				minScanline = Min( minScanline, startY );

#ifdef USE_SSE_OPTIMZATION
				endY = static_cast< unsigned short >( SSE::Ceil( yMax ) );
#else
				endY = static_cast< unsigned short >( std::ceil( yMax ) );
#endif
				endY = Clamp<unsigned short>( endY, startY, maxY );
				maxScanline = Max( maxScanline, endY );

				// dx and dy for an incremental approach
				float dx = v0.Position.X - v1.Position.X;
				float dy = v0.Position.Y - v1.Position.Y;
#ifdef USE_SSE_OPTIMZATION
				float slope = ( dy == 0.0f ) ? 0.0f : SSE::Divide( dx, dy );
#else
				float slope = ( dy == 0.0f ) ? 0.0f : dx / dy;
#endif				

				// Push this element at the selected scanline.
				m_edgeTable[startY].emplace_back( yMax, xMin, xMax, slope, zStart, zEnd );
				validEdgeIndices.push_back( startY );

				// Update next indices.
				v0Index = v1Index;
				++v1Index;
			}

			// Pass if the primitive is culled out.
			if ( isCulled )
			{
				continue;
			}

			// Draw pixels
			GatherPixelsBeingDrawnFromScanlines( aet, minScanline, maxScanline, width, depthBuffering );

			// Clear the current edge table.
			aet.clear();

			size_t size = validEdgeIndices.size();
			for ( size_t i = 0; i < size; ++i )
			{
				unsigned short y = validEdgeIndices[i];
				Assert( y >= 0 && y < m_edgeTable.size() );
				m_edgeTable[y].clear();
			}
			validEdgeIndices.clear();
		}
	}

	void Rasterizer::GatherPixelsBeingDrawnFromScanlines( StlVector<ActiveEdgeTableElement>& aet, unsigned short minScanline, unsigned short maxScanline, unsigned short width, DepthBuffering& depthBuffering ) const
	{
		Assert( m_outputStream );
		Assert( minScanline >= 0 && minScanline <= maxScanline );
		Assert( maxScanline < m_edgeTable.size() );

#ifdef EARLY_Z_CULLING
		// nothing
#else
		Unused( depthBuffering );
#endif

		// for each scanline
		for ( unsigned short y = minScanline; y < maxScanline; ++y )
		{
			if ( !UpdateActiveEdgeTable( aet, y ) )
			{
				continue;
			}

			// Gather pixels being drawn using the AET.
			for ( auto iter = aet.begin(); iter != aet.end(); ++iter )
			{
				// First, select left (=current) and right (=next) edge elements.
				ActiveEdgeTableElement& aetLeft = *iter;
				const EdgeTableElement& etLeft = *aetLeft.ET;
				if ( ++iter == aet.end() )
				{
					aetLeft.CurrentX += etLeft.Slope;
					break;
				}
				ActiveEdgeTableElement& aetRight = *iter;
				const EdgeTableElement& etRight = *aetRight.ET;

				// Approximate intersection pixels betweeen edges on the scanline.
				if ( aetLeft.CurrentX != aetRight.CurrentX )
				{
#ifdef USE_SSE_OPTIMZATION
					unsigned short xLeft = static_cast<unsigned short>( SSE::Trunc( aetLeft.CurrentX ) );
					unsigned short xRight = static_cast<unsigned short>( SSE::Trunc( aetRight.CurrentX ) );
#else
					unsigned short xLeft = static_cast< unsigned short >( std::round( aetLeft.CurrentX ) );
					unsigned short xRight = static_cast< unsigned short >( std::round( aetRight.CurrentX ) );
#endif

					// clipping on RT
					xLeft = Max<unsigned short>( xLeft, 0 );
					xRight = Min<unsigned short>( xRight, width );

					if ( xLeft != xRight )
					{
						// Compute lerp ratio for the left-end pixel and the right-end pixel.
						float diffX0Left = aetLeft.CurrentX - etLeft.XMin;
						float diffX1Left = etLeft.XMax - etLeft.XMin;
						float diffX0Right = aetRight.CurrentX - etRight.XMin;
						float diffX1Right = etRight.XMax - etRight.XMin;
#ifdef USE_SSE_OPTIMZATION
						float lerpRatioLeft = ( diffX1Left == 0.0f ) ? 0.0f : SSE::Divide( diffX0Left, diffX1Left );
						float lerpRatioRight = ( diffX1Right == 0.0f ) ? 0.0f : SSE::Divide( diffX0Right, diffX1Right );
#else
						float lerpRatioLeft = ( diffX1Left == 0.0f ) ? 0.0f : ( diffX0Left / diffX1Left );
						float lerpRatioRight = ( diffX1Right == 0.0f ) ? 0.0f : ( diffX0Right / diffX1Right );
#endif

						// Lerp depth values of L and R between their own edges.
						float depthLeft = Lerp( etLeft.ZStart, etLeft.ZEnd, lerpRatioLeft );
						float depthRight = Lerp( etRight.ZStart, etRight.ZEnd, lerpRatioRight );

						// TODO: other lerp vars...

						// an incremetal approach to lerp depth
						float depthRatioFactor = 1.0f / ( aetRight.CurrentX - aetLeft.CurrentX );
						float ddxDepth = ( depthRight - depthLeft ) * depthRatioFactor;
						float interpolatedDepth = depthLeft;

						// Push inside pixels between intersections into the output stream.
						for ( unsigned short x = xLeft; x < xRight; ++x )
						{
#ifdef EARLY_Z_CULLING
							// depth buffering
							if ( depthBuffering.IsValid() )
							{
								if ( !depthBuffering.Test( x, y, interpolatedDepth ) )
								{
									// Update the next depth value incrementally.
									interpolatedDepth += ddxDepth;
									continue;
								}
							}
#endif

							m_outputStream->Push( x, y, interpolatedDepth );

							// Update the next depth value incrementally.
							interpolatedDepth += ddxDepth;
						}
					}					
				}

				// Update the next position incrementally.
				aetLeft.CurrentX += etLeft.Slope;
				aetRight.CurrentX += etRight.Slope;				
			}
		}
	}

	bool Rasterizer::UpdateActiveEdgeTable( StlVector<ActiveEdgeTableElement>& aet, unsigned short scanline ) const
	{
		Assert( scanline >= 0 && scanline < m_edgeTable.size() );

		float fScanline = static_cast<float>( scanline );

		// Find outside elements in the AET and remove them.
		auto iter = aet.begin();
		while ( iter != aet.end() )
		{
			const ActiveEdgeTableElement& elem = *iter;
			if ( fScanline >= elem.ET->YMax )
			{
				iter = aet.erase( iter );
			}
			else
			{
				++iter;
			}
		}

		// Push inside ET elements on this scanline into the AET.
		const StlVector<EdgeTableElement>& elemList = m_edgeTable[scanline];
		size_t elemSize = elemList.size();
		for ( size_t i = 0; i < elemSize; ++i )
		{
			const EdgeTableElement& elem = elemList[i];
			if ( fScanline < elem.YMax )
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
			// std::sort() is very very slow. :(
			// So, if the size of AET is only two,
			// we reverse AET where node0 > node1 instead of sort().
			// This optimization is meaningful because a triangle mostly haves two edges on a scanline.
			if ( aet.size() == 2 )
			{
				auto& node0 = aet[0];
				auto& node1 = aet[1];
				if ( node0.CurrentX > node1.CurrentX )
				{
					Swap( node0, node1 );
				}
			}
			else
			{
				std::sort( aet.begin(), aet.end() );
			}
		}

		return true;
	}

	void Rasterizer::RasterizeUsingBarycentricCoordinates( const std::shared_ptr<RasterizerInputStream>& inputStream, unsigned short width, unsigned short height )
	{
		Assert( inputStream );

#ifdef EARLY_Z_CULLING
		DepthBuffering depthBuffering( GetContext() );
		depthBuffering.SetWritable( false );	// depth-test only on the rasterizer
#endif

		const int NumVerticesPerPrimitive = 3;

		size_t numVertices = inputStream->Size();
		size_t numPrimitives = numVertices / NumVerticesPerPrimitive;

		// for each primitive
		for ( size_t n = 0; n < numPrimitives; ++n )
		{
			// edge: v0 -> v1
			// v0 = i th vertex
			// v1 = (i + 1) th vertex
			//
			// But if v1 is a vertex of the next primitive,
			// we must hold the first vertex of the current primitive to v1.
			// To keep in simple, we step last -> first -> second -> third ... and last - 1.
			size_t v1Index = n * NumVerticesPerPrimitive;
			size_t v0Index = v1Index + ( NumVerticesPerPrimitive - 1 );

			// Make vertices of the triangle.
			const Vector4& v0NDC = inputStream->GetData( v0Index ).Position;
			const Vector4& v1NDC = inputStream->GetData( v1Index ).Position;
			const Vector4& v2NDC = inputStream->GetData( v1Index + 1 ).Position;

			// face culling
			if ( m_cullMode != CullMode::None )
			{
				// FIXME: CW/CCW
				if ( IsBackFace( v0NDC, v1NDC, v2NDC ) )
				{
					continue;
				}
			}

			// Round NDC X/Y coordinates to map onto screen pixels.
			Vector2i v0( SSE::Trunc( std::round( v0NDC.X ) ), SSE::Trunc( std::round( v0NDC.Y ) ) );
			Vector2i v1( SSE::Trunc( std::round( v1NDC.X ) ), SSE::Trunc( std::round( v1NDC.Y ) ) );
			Vector2i v2( SSE::Trunc( std::round( v2NDC.X ) ), SSE::Trunc( std::round( v2NDC.Y ) ) );

			// Compute a bounding box of the triangle and clip against the screen.
			int minX = Max( Min( v0.X, Min( v1.X, v2.X ) ), 0 );
			int minY = Max( Min( v0.Y, Min( v1.Y, v2.Y ) ), 0 );
			int maxX = Max( Min( Max( v0.X, Max( v1.X, v2.X ) ), width - 1 ), 0 );
			int maxY = Max( Min( Max( v0.Y, Max( v1.Y, v2.Y ) ), height - 1), 0 );
			
			// Compute X/Y incremental moments
			int x01Inc = v0.Y - v1.Y;
			int x12Inc = v1.Y - v2.Y;
			int x20Inc = v2.Y - v0.Y;
			int y01Inc = v1.X - v0.X;
			int y12Inc = v2.X - v1.X;
			int y20Inc = v0.X - v2.X;

			// Compute barycentric coordinates at left-top corner as the start point.
			Vector2i p( minX, minY );
			int w0 = ComputeBarycentric( p, v1, v2 );
			int w1 = ComputeBarycentric( p, v2, v0 );
			int w2 = ComputeBarycentric( p, v0, v1 );
			
			float normalizeBase = 1.0f / static_cast<float>( w0 + w1 + w2 );
			float lamdaZ3[3] = { v0NDC.Z * normalizeBase, v1NDC.Z * normalizeBase, v2NDC.Z * normalizeBase };

			// DX rasterization rule: http://msdn.microsoft.com/en-us/library/windows/desktop/cc627092(v=vs.85).aspx#Triangle
			// A top edge, is an edge that is exactly horizontal and is above the other edges.
			// A left edge, is an edge that is not exactly horizontal and is on the left side of the triangle.
			#define IsTopOrLeft( p0, p1 )	( ( p0.Y == p1.Y && p0.Y == minY ) || ( p0.X == minX || p1.X == minX ) )
			int bias0 = IsTopOrLeft( v1, v2 ) ? -1 : 0;
			int bias1 = IsTopOrLeft( v2, v0 ) ? -1 : 0;
			int bias2 = IsTopOrLeft( v0, v1 ) ? -1 : 0;

			// Rasterze inside pixels of the triangle from the bounding box.
			for ( int y = minY; y <= maxY; ++y )
			{
				int p_w0 = w0;
				int p_w1 = w1;
				int p_w2 = w2;

				float depth = lamdaZ3[0] * p_w0 + lamdaZ3[1] * p_w1 + lamdaZ3[2] * p_w2;
				float depthInc = lamdaZ3[0] * x12Inc + lamdaZ3[1] * x20Inc + lamdaZ3[2] * x01Inc;

				for ( int x = minX; x <= maxX; ++x )
				{
					// Is the pixel in the triangle?
					if ( p_w0 >= bias0 && p_w1 >= bias1 && p_w2 >= bias2 )
					{
#ifdef EARLY_Z_CULLING
						// depth buffering
						if ( depthBuffering.IsValid() )
						{
							if ( !depthBuffering.Test( static_cast<unsigned short>( x ), static_cast<unsigned short>( y ), depth ) )
							{
								continue;
							}
						}
#endif

						m_outputStream->Push( x, y, depth );
					}

					// Increment X-coordinates.
					p_w0 += x12Inc;
					p_w1 += x20Inc;
					p_w2 += x01Inc;
					depth += depthInc;
				}

				// Increment Y-coordinates.
				w0 += y12Inc;
				w1 += y20Inc;
				w2 += y01Inc;
			}
		}		
	}

	void Rasterizer::RasterizeUsingBarycentricCoordinatesSSE( const std::shared_ptr<RasterizerInputStream>& inputStream, unsigned short width, unsigned short height )
	{
		Assert( inputStream );

#ifdef EARLY_Z_CULLING
		DepthBuffering depthBuffering( GetContext() );
		depthBuffering.SetWritable( false );	// depth-test only on the rasterizer
#endif

		const int NumVerticesPerPrimitive = 3;

		size_t numVertices = inputStream->Size();
		size_t numPrimitives = numVertices / NumVerticesPerPrimitive;

		// for each primitive
		for ( size_t n = 0; n < numPrimitives; ++n )
		{
			// edge: v0 -> v1
			// v0 = i th vertex
			// v1 = (i + 1) th vertex
			//
			// But if v1 is a vertex of the next primitive,
			// we must hold the first vertex of the current primitive to v1.
			// To keep in simple, we step last -> first -> second -> third ... and last - 1.
			size_t v1Index = n * NumVerticesPerPrimitive;
			size_t v0Index = v1Index + ( NumVerticesPerPrimitive - 1 );

			// Make vertices of the triangle.
			const Vector4& v0NDC = inputStream->GetData( v0Index ).Position;
			const Vector4& v1NDC = inputStream->GetData( v1Index ).Position;
			const Vector4& v2NDC = inputStream->GetData( v1Index + 1 ).Position;

			xxm128 v0xxm = SSE::LoadUnaligned( v0NDC.Value );
			xxm128 v1xxm = SSE::LoadUnaligned( v1NDC.Value );
			xxm128 v2xxm = SSE::LoadUnaligned( v2NDC.Value );

			// face culling
			if ( m_cullMode != CullMode::None )
			{
				// FIXME: CW/CCW
				if ( SSE::IsBackFace( v0xxm, v1xxm, v2xxm ) )
				{
					continue;
				}
			}

			// Round NDC X/Y coordinates to map onto screen pixels.
			xxm128i v0Rounded = SSE::ReinterpretCast_xxm128i( SSE::Round( v0xxm ) );
			xxm128i v1Rounded = SSE::ReinterpretCast_xxm128i( SSE::Round( v1xxm ) );
			xxm128i v2Rounded = SSE::ReinterpretCast_xxm128i( SSE::Round( v2xxm ) );

			// Compute a bounding box of the triangle and clip against the screen.
			xxm128i minXY = SSE::Max( SSE::Min( v0Rounded, SSE::Min( v1Rounded, v2Rounded ) ), SSE::xxm128i_Zero );
			xxm128i maxXY = SSE::Max( SSE::Max( v0Rounded, SSE::Max( v1Rounded, v2Rounded ) ), SSE::xxm128i_Zero );
			unsigned short minX = static_cast< unsigned short >( minXY.m128i_i32[0] );
			unsigned short minY = static_cast< unsigned short >( minXY.m128i_i32[1] );
			unsigned short maxX = static_cast< unsigned short >( Min( maxXY.m128i_i32[0], width - 1 ) );
			unsigned short maxY = static_cast< unsigned short >( Min( maxXY.m128i_i32[1], height - 1 ) );

			// Compute barycentric coordinates at left-top corner as the start point.
			Vector2i p( minX, minY );
			Vector2i v0( v0Rounded.m128i_i32[0], v0Rounded.m128i_i32[1] );
			Vector2i v1( v1Rounded.m128i_i32[0], v1Rounded.m128i_i32[1] );
			Vector2i v2( v2Rounded.m128i_i32[0], v2Rounded.m128i_i32[1] );
			int w0 = ComputeBarycentric( p, v1, v2 );
			int w1 = ComputeBarycentric( p, v2, v0 );
			int w2 = ComputeBarycentric( p, v0, v1 );

			xxm128 normalizeBase = SSE::Divide( SSE::xxm128_One, SSE::Load( static_cast< float >( w0 + w1 + w2 ) ) );
			xxm128 wZ = SSE::Load( v0xxm.m128_f32[2], v1xxm.m128_f32[2], v2xxm.m128_f32[2], 0.0f );
			wZ = SSE::Multiply( wZ, normalizeBase );

			// Compute X/Y incremental moments
			int A0Inc = v1.Y - v2.Y;
			int A1Inc = v2.Y - v0.Y;
			int A2Inc = v0.Y - v1.Y;

			// Loop vectorization using SSE (drawing four pixels at once).
			// So we prepare 0th to 3rd coordinates in XXM registers.
			xxm128i A0IncEnum = SSE::Load( 0, A0Inc, A0Inc * 2, A0Inc * 3 );
			xxm128i A0Inc4Step4 = SSE::Load( A0Inc * 4 );
			
			xxm128i A1IncEnum = SSE::Load( 0, A1Inc, A1Inc * 2, A1Inc * 3 );
			xxm128i A1Inc4Step4 = SSE::Load( A1Inc * 4 );
			
			xxm128i A2IncEnum = SSE::Load( 0, A2Inc, A2Inc * 2, A2Inc * 3 );
			xxm128i A2Inc4Step4 = SSE::Load( A2Inc * 4 );

			int B0Inc = v2.X - v1.X;
			int B1Inc = v0.X - v2.X;
			int B2Inc = v1.X - v0.X;
			
			// DX rasterization rule: http://msdn.microsoft.com/en-us/library/windows/desktop/cc627092(v=vs.85).aspx#Triangle
			// A top edge, is an edge that is exactly horizontal and is above the other edges.
			// A left edge, is an edge that is not exactly horizontal and is on the left side of the triangle.
			#define IsTopOrLeft( p0, p1 )	( ( p0.Y == p1.Y && p0.Y == minY ) || ( p0.X == minX || p1.X == minX ) )
			xxm128i bias0 = SSE::Load( IsTopOrLeft( v1, v2 ) ? -1 : 0 );
			xxm128i bias1 = SSE::Load( IsTopOrLeft( v2, v0 ) ? -1 : 0 );
			xxm128i bias2 = SSE::Load( IsTopOrLeft( v0, v1 ) ? -1 : 0 );

			// Rasterze inside pixels of the triangle from the bounding box.
			for ( unsigned short y = minY; y <= maxY; ++y )
			{
				xxm128i p4_w0 = SSE::Load( w0 );
				p4_w0 = SSE::Add( p4_w0, A0IncEnum );

				xxm128i p4_w1 = SSE::Load( w1 );
				p4_w1 = SSE::Add( p4_w1, A1IncEnum );

				xxm128i p4_w2 = SSE::Load( w2 );
				p4_w2 = SSE::Add( p4_w2, A2IncEnum );

				xxm128 z4 = SSE::Add( SSE::Multiply( SSE::Load( wZ.m128_f32[0] ), SSE::ReinterpretCast_xxm128( p4_w0 ) ),
								SSE::Add( SSE::Multiply( SSE::Load( wZ.m128_f32[1] ), SSE::ReinterpretCast_xxm128( p4_w1 ) ),
											SSE::Multiply( SSE::Load( wZ.m128_f32[2] ), SSE::ReinterpretCast_xxm128( p4_w2 ) ) ) );
				xxm128 zInc4Step4 = SSE::Load( ( wZ.m128_f32[0] * A0Inc + wZ.m128_f32[1] * A1Inc + wZ.m128_f32[2] * A2Inc ) * 4.0f );

				for ( unsigned short x = minX; x <= maxX; x += 4,
												// Increment X-coordinates.
												p4_w0 = SSE::Add( p4_w0, A0Inc4Step4 ),
												p4_w1 = SSE::Add( p4_w1, A1Inc4Step4 ),
												p4_w2 = SSE::Add( p4_w2, A2Inc4Step4 ),
												z4 = SSE::Add( z4, zInc4Step4 ) )
				{
					// Is the pixel in the triangle?
					xxm128i test0 = SSE::Less( bias0, p4_w0 );
					xxm128i test1 = SSE::Less( bias1, p4_w1 );
					xxm128i test2 = SSE::Less( bias2, p4_w2 );
					xxm128i testAll = SSE::And( SSE::And( test0, test1 ), test2 );

					// Early out this quad.
					if ( SSE::IsAllZero( testAll ) )
					{
						continue;
					}

					// Push four pixels being shaded into the output stream.
					for ( unsigned short p = 0; p < 4; ++p )
					{
						if ( testAll.m128i_i32[p] )
						{
#ifdef EARLY_Z_CULLING
							// depth buffering
							if ( depthBuffering.IsValid() )
							{
								if ( !depthBuffering.Test( x, y, z4.m128_f32[p] ) )
								{
									continue;
								}
							}
#endif

							// Clip against the screen.
							unsigned short px = x + p;
							if ( px >= width )
							{
								break;
							}

							m_outputStream->Push( px, y, z4.m128_f32[p] );
						}
					}					
				}

				// Increment Y-coordinates.
				w0 += B0Inc;
				w1 += B1Inc;
				w2 += B2Inc;
			}
		}
	}

	void Rasterizer::TransformViewport( const std::shared_ptr<RasterizerInputStream>& inputStream, unsigned short width, unsigned short height ) const
	{
		// viewport transform for projective coordinates
		if ( inputStream->GetCoordinatesType() != CoordinatesType::Projective )
		{
			return;
		}

		float factorX = width * 0.5f;
		float factorY = height * 0.5f;

#ifdef USE_SSE_OPTIMZATION
		xxm128 toViewportMul = SSE::Load( factorX, -factorY, 0.5f, 1.0f );
		xxm128 toViewportAdd = SSE::Load( factorX, factorY, 0.5f, 0.0f );
#endif

		size_t numVertices = inputStream->Size();
		for ( size_t v = 0; v < numVertices; ++v )
		{
			auto& data = inputStream->GetData( v );

#ifdef USE_SSE_OPTIMZATION
			// perspective division
			xxm128 position = SSE::LoadUnaligned( data.Position.Value );
			xxm128 divider = Swizzle( position, SWIZZLE_MASK( 3, 3, 3, 3 ) );
			position = SSE::Divide( position, divider );

			// NDC -> viewport
			position = SSE::Multiply( position, toViewportMul );
			position = SSE::Add( position, toViewportAdd );
			SSE::StoreUnaligned( data.Position.Value, position );
#else
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
#endif
		}	
	}
		

	/* class PixelShader
	*/
	std::shared_ptr<PixelShaderOutputStream> PixelShader::Process( const std::shared_ptr<PixelShaderInputStream>& inputStream )
	{
		Assert( inputStream );

		m_outputStream->Clear();

		size_t inputStreamSize = inputStream->Size();
		if ( inputStreamSize <= 0 )
		{
			return m_outputStream;
		}

		if ( m_outputStream->Capacity() < inputStreamSize )
		{
			m_outputStream->Reserve( inputStreamSize );
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
			
			// TODO: pixel shading
			//
			m_outputStream->Push( fragment, color );
		}
#endif

		return m_outputStream;
	}


	/* class OutputMerger
	*/
	std::shared_ptr<OutputMergerOutputStream> OutputMerger::Process( const std::shared_ptr<OutputMergerInputStream>& inputStream )
	{
		Assert( inputStream );

		// If the output merger has a UAV, merge input stream into the UAV and queue a resolving task.
		if ( m_uav )
		{
			m_uav->Merge( inputStream );

			// Queue a resolving task.
			auto task = std::bind( 
				[=]()
				{
					LockGuard<Mutex> guard( m_uav->GetLock() );
					Resolve( m_uav->GetStreamSource() );
					m_uav->Clear();
				} );
			ThreadPool::GetInstance()->Queue( task );

			return nullptr;
		}

		// Otherwise, resolve the input stream.
		Resolve( inputStream );

		return m_outputStream;
	}

	void OutputMerger::Resolve( const std::shared_ptr<OutputMergerInputStream>& inputStream )
	{
		VerifyReentry();

		size_t inputStreamSize = inputStream->Size();
		if ( inputStreamSize <= 0 )
		{
			return;
		}
		
		// UNDONE: Currently, we assume RTs is only one.
		Assert( ( GetContext()->NumberOfRenderTargets() == 1 ) && "MRT is not implemented yet" );
		std::shared_ptr<Texture> rt = GetContext()->GetRenderTaget( 0 );
		if ( rt == nullptr )
		{
			return;
		}

		// Get raw memory from the render target.
		LockGuardPtr<Texture> guardRT( rt );
		byte* bufferRT = static_cast< byte* >( guardRT.Ptr() );
		if ( bufferRT == nullptr )
		{
			return;
		}

		int widthRT = rt->Width();
		int strideRT = GetBytesPerPixel( rt->Format() );
		

		// Now, write color on render targets and the depth stencil buffer here.
		DepthBuffering depthBuffering( GetContext() );
		for ( size_t i = 0; i < inputStreamSize; ++i )
		{
			const auto& fragment = inputStream->GetDataConst( i );

			// Late depth buffering.
			if ( depthBuffering.IsValid() && 
				!depthBuffering.Test( fragment ) )
			{
				continue;
			}

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
	}
};
