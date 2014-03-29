// Include Directive
// 
#include "../kihx/kihx.h"
#include "../woocom/WModule.h"
#include "../xtozero/xtozero.h"
#include "../utility/math3d.h"

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <memory.h>
#include <type_traits>
#include <string>

#include "glut.h"


// Constants Directive
//
#define	SCREEN_WIDTH 640
#define	SCREEN_HEIGHT 480
#define COLOR_DEPTH 3


// Global variables
//
typedef unsigned char byte;
byte g_pppScreenImage[SCREEN_HEIGHT][SCREEN_WIDTH][COLOR_DEPTH];


// Static variables
//
static GLdouble g_dZoomFactor = 1.0;
static GLint g_iHeight;
std::string g_meshFileName = "cube.ply";

// Customization 
//
namespace
{
	enum class TransformType
	{
		World = 0,
		View = 1,
		Projection = 2
	};
	

	typedef void( *FnLoadMeshFromFile)( const char* filename );
	typedef void( *FnRenderToBuffer)( void* buffer, int width, int height, int bpp );
	typedef void( *FnSetTransform)( int transformType, const float* matrix4x4 );


	// 지정된 모듈로부터 함수 주소 얻어오기
	template<typename FuncPtr>
	FuncPtr GetFunctionFromModule( HMODULE hModule, const char* functionName )
	{
		static_assert( std::is_pointer<FuncPtr>::value, "not pointer" );
		static_assert( sizeof( FuncPtr ) > 0, "unknown type" );

		FuncPtr func = nullptr;
		__try
		{
			if ( hModule && functionName )
			{
				func = reinterpret_cast<FuncPtr>( GetProcAddress( hModule, functionName ) );
			}
		}
		__except ( EXCEPTION_EXECUTE_HANDLER )
		{
			// do nothing
		}

		return func;
	};	
			
	// 모듈별 작업 수행
	class ModuleContext
	{
	public:
		ModuleContext() : 
			m_hModule( nullptr ),
			m_fnLoadMeshFromFile( nullptr ),
			m_fnRenderToBuffer( nullptr ),
			m_fnSetTransform( nullptr )
		{
		}

		~ModuleContext()
		{
			Release();
		}

		// 다른 모듈이 로드되면 이전 모듈은 해제한다
		bool Load( const char* moduleName )
		{
			HMODULE h = GetModuleHandle( moduleName );
			if ( h == nullptr )
			{
				h = LoadLibrary( moduleName );
			}
			AssignOrReplace( h );
			return h != nullptr;
		}

		HMODULE& operator=( HMODULE h )
		{
			AssignOrReplace( h );
			return m_hModule;
		}

		HMODULE Get()
		{
			return m_hModule;
		}

		void LoadMeshFromFile( const char* filename )
		{
			if ( m_fnLoadMeshFromFile )
			{
				m_fnLoadMeshFromFile( filename );
			}
		}

		void RenderToBuffer( byte* buffer )
		{
			if ( m_fnRenderToBuffer )
			{
				m_fnRenderToBuffer( buffer, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_DEPTH * 8 );
			}
		}

		void SetTransform( TransformType transformType, const float* matrix4x4 )
		{
			if ( m_fnSetTransform )
			{
				m_fnSetTransform( static_cast<int>( transformType ), matrix4x4 );
			}
		}

		// install module function
#define SETUP_FUNC( func_name )	\
		void InstallFunction##func_name( const char* functionName )	\
		{	\
			m_fn##func_name = GetFunctionFromModule<Fn##func_name>( m_hModule, functionName );	\
			glutPostRedisplay();	\
		}

		SETUP_FUNC( LoadMeshFromFile );
		SETUP_FUNC( RenderToBuffer );
		SETUP_FUNC( SetTransform );

	private:
		void AssignOrReplace( HMODULE h )
		{
			if ( m_hModule == h )
			{
				return;
			}
			
			if ( m_hModule )
			{
				Release();
			}
			m_hModule = h;
		}

		void Release() 
		{
			FreeLibrary( m_hModule );
			m_fnLoadMeshFromFile = nullptr;
			m_fnRenderToBuffer = nullptr;	
			m_fnSetTransform = nullptr;
		}

	private:
		HMODULE m_hModule;
		FnLoadMeshFromFile m_fnLoadMeshFromFile;
		FnRenderToBuffer m_fnRenderToBuffer;
		FnSetTransform m_fnSetTransform;
	};

	ModuleContext g_ModuleContext;
};


static void LoadModuleKihx()
{
#ifdef NDEBUG
	const char* ModuleName = "kihx.dll";
#else
	const char* ModuleName = "kihxD.dll";
#endif	
	
	if ( !g_ModuleContext.Load( ModuleName ) )
	{
		printf( "Load module failure: %s\n", ModuleName );
		return;
	}
	
	g_ModuleContext.InstallFunctionLoadMeshFromFile( "kiLoadMeshFromFile" );
	g_ModuleContext.InstallFunctionRenderToBuffer( "kiRenderToBuffer" );
	g_ModuleContext.InstallFunctionSetTransform( "kiSetTransform" );

	printf( "\n<kihx>\n\n" );
}

static void LoadModuleXTZ()
{
#ifdef _DEBUG
	const char* ModuleName = "xtozeroD.dll";
#else
	const char* ModuleName = "xtozero.dll";
#endif	
	
	if ( !g_ModuleContext.Load( ModuleName ) )
	{
		printf( "Load module failure: %s\n", ModuleName );
		return;
	}
	
	g_ModuleContext.InstallFunctionLoadMeshFromFile( "XtzLoadMeshFromFile" );
	g_ModuleContext.InstallFunctionRenderToBuffer( "XtzRenderToBuffer" );

	printf( "\n<xtozero>\n\n" );
}

static void LoadModuleWoocom()
{
#ifdef _DEBUG
	const char* ModuleName = "woocomD.dll";
#else
	const char* ModuleName = "woocomD.dll";
#endif	

	if ( !g_ModuleContext.Load( ModuleName ) )
	{
		printf( "Load module failure: %s\n", ModuleName );
		return;
	}

	g_ModuleContext.InstallFunctionLoadMeshFromFile( "WLoadMesh" );
	g_ModuleContext.InstallFunctionRenderToBuffer( "WRender" );

	printf( "\n<woocom>\n\n" );
}

static void LoadModuleCoolD()
{
#ifdef _DEBUG
	const char* ModuleName = "cooldD.dll";
#else
	const char* ModuleName = "cooldR.dll";
#endif	

	if ( !g_ModuleContext.Load( ModuleName ) )
	{
		printf( "Load module failure: %s\n", ModuleName );
		return;
	}

	g_ModuleContext.InstallFunctionLoadMeshFromFile( "coold_LoadMeshFromFile" );
	g_ModuleContext.InstallFunctionRenderToBuffer( "coold_RenderToBuffer" );
	g_ModuleContext.InstallFunctionSetTransform( "coold_SetTransform" );

	printf( "\n<CoolD>\n\n" );
}

//-----------------------------------------------------------------------------------------------------------------------
//
// TODO THIS...
//
//-----------------------------------------------------------------------------------------------------------------------
void SetupTransform()
{
	const float PI = 3.141592654f;

    // For our world matrix, we will just rotate the object about the y-axis.
    Matrix4 matWorld;

    // Set up the rotation matrix to generate 1 full rotation (2*PI radians) 
    // every 1000 ms. To avoid the loss of precision inherent in very high 
    // floating point numbers, the system time is modulated by the rotation 
    // period before conversion to a radian angle.
 //   unsigned int iTime = timeGetTime() % 1000;
 //   float fAngle = iTime * ( 2.0f * PI ) / 1000.0f;
	//matWorld.RotateY( fAngle );

	Matrix4 matScale;
	matScale.Scaling( 10.0f, 10.0f, 10.0f );
	matWorld = matWorld * matScale;
    g_ModuleContext.SetTransform( TransformType::World, matWorld.M );

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. Here, we set the
    // eye five units back along the z-axis and up three units, look at the
    // origin, and define "up" to be in the y-direction.
    Vector3 vEyePt( 1.5f, 3.0f, -5.0f );
    Vector3 vLookatPt( 0.0f, 0.0f, 0.0f );
    Vector3 vUpVec( 0.0f, 1.0f, 0.0f );
    Matrix4 matView;
    matView.LookAtLH( vEyePt, vLookatPt, vUpVec );
    g_ModuleContext.SetTransform( TransformType::View, matView.M );

    // For the projection matrix, we set up a perspective transform (which
    // transforms geometry from 3D view space to 2D viewport space, with
    // a perspective divide making objects smaller in the distance). To build
    // a perpsective transform, we need the field of view (1/4 pi is common),
    // the aspect ratio, and the near and far clipping planes (which define at
    // what distances geometry should be no longer be rendered).
    Matrix4 matProj;
    matProj.PerspectiveLH( PI / 4.0f, 1.0f, 1.0f, 100.0f );
    g_ModuleContext.SetTransform( TransformType::Projection, matProj.M );
}

void makeCheckImage( void)
{
	SetupTransform();

	g_ModuleContext.RenderToBuffer( (byte*) g_pppScreenImage );
}


//-----------------------------------------------------------------------------------------------------------------------
//
// DO NOT EDIT!!
//
//-----------------------------------------------------------------------------------------------------------------------
void init( void)
{	
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glShadeModel( GL_FLAT );

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
}

void display( void)
{
	glClear( GL_COLOR_BUFFER_BIT );

	makeCheckImage();
	glRasterPos2i( 0, 480 );
	glPixelZoom( 1.f, -1.f );
	glDrawPixels( SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, g_pppScreenImage );
	glFlush();
}

void reshape( int w, int h)
{
	glViewport( 0, 0,( GLsizei) w,( GLsizei) h );
	g_iHeight =( GLint) h;
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0.0,( GLdouble) w, 0.0,( GLdouble) h );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

void motion( int x, int y)
{
	printf( "[motion] x: %d, y: %d\n", x, y );

	static GLint screeny;

	screeny = g_iHeight -( GLint) y;
	glRasterPos2i( x, screeny );
	//glPixelZoom( g_dZoomFactor, g_dZoomFactor );
	glCopyPixels( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_COLOR );
	glPixelZoom( 1.0, 1.0 );
	glFlush();
}

void ReloadMesh()
{
	g_ModuleContext.LoadMeshFromFile( g_meshFileName.c_str() );
}

void keyboard( unsigned char key, int x, int y)
{
	printf( "[keyboard] key: %c\n", key );
	
	HMODULE hCurrentModule = nullptr;

	switch( key )
	{
	case '1':
		LoadModuleKihx();
		ReloadMesh();
		break;

	case '2':
		LoadModuleWoocom();
		ReloadMesh();
		break;

	case '3':
		LoadModuleXTZ();
		ReloadMesh();
		break;

	case '4':
		LoadModuleCoolD();
		ReloadMesh();
		break;

	case '8':
		g_meshFileName = "input.msh";
		ReloadMesh();
		break;

	case '9':
		g_meshFileName = "cube.ply";
		ReloadMesh();
		break;

	case '0':
		g_meshFileName = "object.ply";
		ReloadMesh();
		break;

	case 'r':
	case 'R':
		g_dZoomFactor = 1.0;
		glutPostRedisplay();
		printf( "g_dZoomFactor reset to 1.0\n" );
		break;

	case 'z':
		g_dZoomFactor += 0.5;
		if ( g_dZoomFactor >= 5.0) 
		{
			g_dZoomFactor = 5.0;
		}
		printf( "g_dZoomFactor is now %4.1f\n", g_dZoomFactor );
		break;

	case 'Z':
		g_dZoomFactor -= 0.5;
		if ( g_dZoomFactor <= 0.5) 
		{
			g_dZoomFactor = 0.5;
		}
		printf( "g_dZoomFactor is now %4.1f\n", g_dZoomFactor );
		break;

	case 27:
		exit( 0 );
		break;

	default:
		break;
	}
}

int main( int argc, char** argv)
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGBA );
	glutInitWindowSize( SCREEN_WIDTH, SCREEN_HEIGHT );
	glutInitWindowPosition( 100, 100 );
	glutCreateWindow( argv[0] );
	init();
	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );
	glutMotionFunc( motion );
	glutMainLoop();
	return 0; 
}

