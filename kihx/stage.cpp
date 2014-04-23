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
#ifdef DEPTHFUNC_LAMDA
		m_context( context ),
#endif
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

	template<class T>
	bool DepthBuffering::Execute( unsigned short x, unsigned short y, T depth )
	{
		VerifyReentry();

		Assert( IsValid() );
		Assert( x >= 0 && x < m_width );
		Assert( y >= 0 && y < m_ds->Height() );

		T& dst = GetValueRef<T>( x, y );

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
		if ( m_depthWritable )
		{
			dst = depth;
		}

		return true;
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
		DoScanlineConversion( inputStream, m_outputStream, width, height );

		return m_outputStream;
	}

	void Rasterizer::DoScanlineConversion( const std::shared_ptr<RasterizerInputStream>& inputStream, std::shared_ptr<RasterizerOutputStream> outputStream, unsigned short width, unsigned short height )
	{
		Assert( inputStream );
		Assert( outputStream );
		
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

#ifdef EARLY_Z_CULLING
		DepthBuffering depthBuffering( GetContext() );
		depthBuffering.SetWritable( false );	// depth-test only on the rasterizer
#else
		DepthBuffering depthBuffering( nullptr );
#endif

		// Rasterize primitives
		if ( numVerticesPerPrimitive == 3 )
		{
			RasterizeUsingBarycentricCoordinates( inputStream, outputStream, width, height, numVertices, depthBuffering );
		}
		else
		{
			RasterizeUsingEdgeTable( inputStream, outputStream, width, height, numVerticesPerPrimitive, numVertices, depthBuffering );
		}
	}

	void Rasterizer::RasterizeUsingEdgeTable( const std::shared_ptr<RasterizerInputStream>& inputStream, std::shared_ptr<RasterizerOutputStream> outputStream, unsigned short width, unsigned short height, size_t numVerticesPerPrimitive, size_t numVertices, DepthBuffering& depthBuffering )
	{
		// Reserve ET space based on scanlines.
		m_edgeTable.clear();
		m_edgeTable.resize( height );

		// the list of indices of edges in ET to reduce traversing empty elements.
		std::vector<unsigned short> validEdgeIndices;
		validEdgeIndices.reserve( numVerticesPerPrimitive );

		// active edge table
		std::vector<ActiveEdgeTableElement> aet;
		aet.reserve( 8 );

		// Build an edge table by traversing each primitive in vertices,
		// and draw each primitive.
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

#if 0
			const auto& v0 = inputStream->GetData( v0Index );
			const auto& v1 = inputStream->GetData( v1Index );
			const auto& v2 = inputStream->GetData( v1Index + 1 );

			// face culling
			if ( m_cullMode != CullMode::None )
			{
				// FIXME: CW/CCW
				if ( IsBackFaceSSE( v0.Position, v1.Position, v2.Position ) )
				{
					continue;
				}
			}

			const auto& v3 = inputStream->GetData( v1Index + 2 );
			const auto& v4 = inputStream->GetData( v1Index + 3 );
			const auto& v5 = inputStream->GetData( v1Index + 4 );	// quad???

			// Make an ET element.

			//float xMax = Max( v0.Position.X, v1.Position.X );
			//float xMin = Min( v0.Position.X, v1.Position.X );
			SSE::XXM128 xMaxCandidate4 = SSE::XXM128_Load( v0.Position.X, v1.Position.X, v2.Position.X, v3.Position.X );
			SSE::XXM128 xMinCandidate4 = SSE::XXM128_Load( v1.Position.X, v2.Position.X, v3.Position.X, v4.Position.X );
			SSE::XXM128 xMin4 = SSE::XXM128_Min( xMinCandidate4, xMaxCandidate4 );
			SSE::XXM128 xMax4 = SSE::XXM128_Max( xMinCandidate4, xMaxCandidate4 );

			//float zStart = v1.Position.Z;	// Select xMin's Z.
			//float zEnd = v0.Position.Z;
			//if ( v0.Position.X < v1.Position.X )
			//{
			//	Swap( zStart, zEnd );
			//}
			SSE::XXM128 zStart4 = SSE::XXM128_Load( v1.Position.Z, v2.Position.Z, v3.Position.Z, v4.Position.Z );
			SSE::XXM128 zEnd4 = SSE::XXM128_Load( v0.Position.Z, v1.Position.Z, v2.Position.Z, v3.Position.Z );

			SSE::XXM128 less4 = SSE::XXM128_Less( xMaxCandidate4, xMinCandidate4 );

			//float yMax = Max( v0.Position.Y, v1.Position.Y );
			//float yMin = Min( v0.Position.Y, v1.Position.Y );
			SSE::XM128 yMaxCandidate4 = SSE::XXM128_Load( v0.Position.Y, v1.Position.Y, v2.Position.Y, v3.Position.Y );
			SSE::XXM128 yMinCandidate4 = SSE::XXM128_Load( v1.Position.Y, v2.Position.Y, v3.Position.Y, v4.Position.Y );
			SSE::XXM128 yMin4 = SSE::XXM128_Min( yMinCandidate4, yMaxCandidate4 );
			SSE::XXM128 yMax4 = SSE::XXM128_Max( yMinCandidate4, yMaxCandidate4 );

			// Select start and end scanlines with Y-axis clipping.
			startY = static_cast< unsigned short >( SSE::Ceil( yMin ) );
			startY = Clamp<unsigned short>( startY, 0, maxY );
			minScanline = Min( minScanline, startY );

			endY = static_cast< unsigned short >( SSE::Ceil( yMax ) );
			endY = Clamp<unsigned short>( endY, startY, maxY );
			maxScanline = Max( maxScanline, endY );

			// dx and dy for an incremental approach
			float dx = v0.Position.X - v1.Position.X;
			float dy = v0.Position.Y - v1.Position.Y;
			float slope = ( dy == 0.0f ) ? 0.0f : SSE::Divide( dx, dy );

			// Push this element at the selected scanline.
			m_edgeTable[startY].emplace_back( yMax, xMin, xMax, slope, zStart, zEnd );
			validEdgeIndices.push_back( startY );

			// Update next indices.
			v0Index = v1Index;
			++v1Index;
#else

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
					if ( IsBackFaceSSE( v0.Position, v1.Position, v2.Position ) )
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
#endif /// 1

			// Pass if the primitive is culled out.
			if ( isCulled )
			{
				continue;
			}

			// Draw pixels
			GatherPixelsBeingDrawnFromScanlines( outputStream, aet, minScanline, maxScanline, width, depthBuffering );

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

	void Rasterizer::GatherPixelsBeingDrawnFromScanlines( std::shared_ptr<RasterizerOutputStream> outputStream, std::vector<ActiveEdgeTableElement>& aet, unsigned short minScanline, unsigned short maxScanline, unsigned short width, DepthBuffering& depthBuffering ) const
	{
		Assert( outputStream );
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
								if ( !depthBuffering.Execute( x, y, interpolatedDepth ) )
								{
									// Update the next depth value incrementally.
									interpolatedDepth += ddxDepth;
									continue;
								}
							}
#endif

							outputStream->Push( x, y, interpolatedDepth );

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

	bool Rasterizer::UpdateActiveEdgeTable( std::vector<ActiveEdgeTableElement>& aet, unsigned short scanline ) const
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
		const std::vector<EdgeTableElement>& elemList = m_edgeTable[scanline];
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

	void Rasterizer::RasterizeUsingBarycentricCoordinates( const std::shared_ptr<RasterizerInputStream>& inputStream, std::shared_ptr<RasterizerOutputStream> outputStream, unsigned short width, unsigned short height, size_t numVertices, DepthBuffering& depthBuffering )
	{
		const int NumVerticesPerPrimitive = 3;

		// Build an edge table by traversing each primitive in vertices,
		// and draw each primitive.
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
#ifdef USE_SSE_OPTIMZATION
				if ( IsBackFaceSSE( v0NDC, v1NDC, v2NDC ) )
#else
				if ( IsBackFace( v0.Position, v1.Position, v2.Position ) )
#endif
				{
					continue;
				}
			}

			// Round NDC X/Y coordinates to map to screen pixels.
#ifdef USE_SSE_OPTIMZATION
			Vector2i v0( SSE::Round( v0NDC.X ), SSE::Round( v0NDC.Y ) );
			Vector2i v1( SSE::Round( v1NDC.X ), SSE::Round( v1NDC.Y ) );
			Vector2i v2( SSE::Round( v2NDC.X ), SSE::Round( v2NDC.Y ) );
#else
			Vector2i v0( std::round( v0NDC.X ), std::round( v0NDC.Y ) );
			Vector2i v1( std::round( v1NDC.X ), std::round( v1NDC.Y ) );
			Vector2i v2( std::round( v2NDC.X ), std::round( v2NDC.Y ) );
#endif

			// Compute a bounding box of the triangle and clip against the screen.
			int minX = Max( Min( v0.X, Min( v1.X, v2.X ) ), 0 );
			int minY = Max( Min( v0.Y, Min( v1.Y, v2.Y ) ), 0 );
			int maxX = Min( Max( v0.X, Max( v1.X, v2.X ) ), width - 1 );
			int maxY = Min( Max( v0.Y, Max( v1.Y, v2.Y ) ), height - 1);
			
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

				for ( int x = minX; x <= maxX; ++x )
				{
					// Is the pixel in the triangle?
					if ( p_w0 >= bias0 && p_w1 >= bias1 && p_w2 >= bias2 )
					{
						float depth = lamdaZ3[0] * p_w0 + lamdaZ3[1] * p_w1 + lamdaZ3[2] * p_w2;

#ifdef EARLY_Z_CULLING
						// depth buffering
						if ( depthBuffering.IsValid() )
						{
							if ( !depthBuffering.Execute( x, y, interpolatedDepth ) )
							{
								continue;
							}
						}
#else
						Unused( depthBuffering );
#endif

						outputStream->Push( x, y, depth );
					}

					// Increment X-coordinates.
					p_w0 += x12Inc;
					p_w1 += x20Inc;
					p_w2 += x01Inc;
				}

				// Increment Y-coordinates.
				w0 += y12Inc;
				w1 += y20Inc;
				w2 += y01Inc;
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
		SSE::XXM128 toViewportMul = SSE::XXM128_Load( factorX, -factorY, 0.5f, 1.0f );
		SSE::XXM128 toViewportAdd = SSE::XXM128_Load( factorX, factorY, 0.5f, 0.0f );
#endif

		size_t numVertices = inputStream->Size();
		for ( size_t v = 0; v < numVertices; ++v )
		{
			auto& data = inputStream->GetData( v );

#ifdef USE_SSE_OPTIMZATION
			// perspective division
			SSE::XXM128 position = SSE::XXM128_LoadUnaligned( data.Position.Value );
			SSE::XXM128 divider = SSE::XXM128_Load( data.Position.W );
			position = SSE::XXM128_Divide( position, divider );

			// NDC -> viewport
			position = SSE::XXM128_Multiply( position, toViewportMul );
			position = SSE::XXM128_Add( position, toViewportAdd );
			SSE::XXM128_StoreUnaligned( data.Position.Value, position );
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

		// If the output merger has a UAV, just merge input stream into the UAV.
		if ( m_uav )
		{
			m_uav->Merge( inputStream );
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

		DepthBuffering depthBuffering( GetContext() );

		// Now, write color on render targets and the depth stencil buffer here.
		for ( size_t i = 0; i < inputStreamSize; ++i )
		{
			const auto& fragment = inputStream->GetDataConst( i );

			// late depth buffering
			if ( depthBuffering.IsValid() )
			{
				if ( !depthBuffering.Execute( fragment.PX, fragment.PY, fragment.Depth ) )
				{
					continue;
				}
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
