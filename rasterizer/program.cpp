// Include Directive
// 
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <memory.h>
#include <type_traits>

#include "glut.h"

#include "../kihx/kihx.h"
#include "../woocom/WModule.h"
#include "../xtozero/xtozero.h"

// Constants Directive
//
#define	SCREEN_WIDTH 640
#define	SCREEN_HEIGHT 480
#define COLOR_DEPTH 3

#define KIHX 1
#define WOOCOM 2
#define XTOZERO 3
#define COOLD 4

//------------Output FileName & LineNumber Show-----------------
#define STR2(x) #x
#define STR(x) STR2(x)
#define MSG(desc) message(__FILE__ "(" STR(__LINE__) "):" #desc)
#define FixLater(desc) __pragma(MSG(desc))
//--------------------------------------------------------------

// Global variables
//
typedef unsigned char ubyte;
ubyte g_pppScreenImage[SCREEN_HEIGHT][SCREEN_WIDTH][COLOR_DEPTH];
int g_selectModule = 0;

// Static variables
//
static GLdouble g_dZoomFactor = 1.0;
static GLint g_iHeight;


// Customization 
//
namespace
{
	typedef void( *FnLoadMeshFromFile)( const char* filename );
	typedef void( *FnRenderToBuffer)( void* buffer, int width, int height, int bpp );
	typedef void( *FnClearColorBuffer)( void* pImage, int width, int height, unsigned long clearColor );

	static FnLoadMeshFromFile g_FnLoadMeshFromFile = NULL;
	static FnRenderToBuffer g_FnRenderToBuffer = NULL;
	static FnClearColorBuffer g_FnClearColorBuffer = NULL;


	// 클래스 복사 방지 기법
	class Uncopyable
	{
	protected:
		Uncopyable() {}
		~Uncopyable() {}

	private:
		Uncopyable( const Uncopyable& );
		Uncopyable& operator=( const Uncopyable& );
	};
	

	// 모듈 리소스 관리 객체
	// 다른 모듈이 로드되면 이전 모듈은 해제한다
	class ModuleLoader : private Uncopyable
	{
	public:
		bool Load( const char* moduleName )
		{
			HMODULE h = GetModuleHandle( moduleName );
			if ( h == NULL )
			{
				h = LoadLibrary( moduleName );
			}
			AssignOrReplace( h );
			return h != NULL;
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

	private:
		void AssignOrReplace( HMODULE h )
		{
			if ( m_hModule == h )
			{
				return;
			}

			if ( m_hModule )
			{
				FreeLibrary( m_hModule );
			}
			m_hModule = h;
		}

	private:
		HMODULE m_hModule;
	};

	ModuleLoader g_hModule;

	// 지정된 모듈로부터 함수 주소 얻어오기
	template<typename FuncPtr>
	FuncPtr GetFunctionFromModule( HMODULE hModule, const char* functionName )
	{
		static_assert( std::is_pointer<FuncPtr>::value, "not pointer" );
		static_assert( sizeof( FuncPtr ) > 0, "unknown type" );

		__try
		{
			if ( hModule && functionName )
			{
				return (FuncPtr) GetProcAddress( hModule, functionName );
			}
		}
		__except ( EXCEPTION_EXECUTE_HANDLER )
		{
			// do nothing
		}
	
		return NULL;
	};


	// 래스터라이저 기능 수행
	inline void LoadMeshFromFile( const char* filename )
	{
		if ( g_FnLoadMeshFromFile )
		{
			g_FnLoadMeshFromFile( filename );
		} 
	}

	inline void RenderToBuffer()
	{
		if ( g_FnRenderToBuffer )
		{
			g_FnRenderToBuffer( g_pppScreenImage, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_DEPTH * 8 );
		}
	}

	inline void ClearBuffer()
	{
		if( g_FnClearColorBuffer )
		{
			g_FnClearColorBuffer( g_pppScreenImage, SCREEN_WIDTH, SCREEN_HEIGHT, 0xff00ffff );
		}		
	}	

	/* 템플릿을 활용한 런타임 함수 로더
	*/
	void InstallFunctionLoadMeshFromFile( HMODULE hModule, const char* functionName )
	{
		g_FnLoadMeshFromFile = GetFunctionFromModule<FnLoadMeshFromFile>( hModule, functionName );

		LoadMeshFromFile( "input.msh" );
	}

	void InstallFunctionRenderToBuffer( HMODULE hModule, const char* functionName )
	{
		g_FnRenderToBuffer = GetFunctionFromModule<FnRenderToBuffer>( hModule, functionName );


		glutPostRedisplay();
	}

	/* 하위 호환성을 위해 유지
	*/
	void InstallFunctionLoadMeshFromFile( FnLoadMeshFromFile fp )
	{
		g_FnLoadMeshFromFile = fp;
		LoadMeshFromFile( "input.msh" );
	}

	void InstallFunctionRenderToBuffer( FnRenderToBuffer fp )
	{
		g_FnRenderToBuffer = fp;
		glutPostRedisplay();
	}

	void InstallFunctionClearColorBuffer( FnClearColorBuffer fp )
	{
		g_FnClearColorBuffer = fp;
		glutPostRedisplay();
	}
};


static void LoadModuleKihx()
{
#ifdef NDEBUG
	const char* ModuleName = "kihx.dll";
#else
	const char* ModuleName = "kihxD.dll";
#endif	
	
	if ( !g_hModule.Load( ModuleName ) )
	{
		printf( "Load module failure: %s\n", ModuleName );
		return;
	}
	
	InstallFunctionLoadMeshFromFile( g_hModule.Get(), "kiLoadMeshFromFile" );
	InstallFunctionRenderToBuffer( g_hModule.Get(), "kiRenderToBuffer" );

	printf( "\n<kihx>\n\n" );

	g_selectModule = KIHX;
}

static void LoadModuleXTZ()
{
#ifdef _DEBUG
	const char* ModuleName = "xtozeroD.dll";
#else
	const char* ModuleName = "xtozero.dll";
#endif	
	
	if ( !g_hModule.Load( ModuleName ) )
	{
		printf( "Load module failure: %s\n", ModuleName );
		return;
	}
	
	InstallFunctionLoadMeshFromFile( g_hModule.Get(), "XtzLoadMeshFromFile" );
	InstallFunctionRenderToBuffer( g_hModule.Get(), "XtzRenderToBuffer" );

	printf( "\n<xtozero>\n\n" );

	g_selectModule = XTOZERO;
}

//-----------------------------------------------------------------------------------------------------------------------
//
// TODO THIS...
//
//-----------------------------------------------------------------------------------------------------------------------
void makeCheckImage( void)
{
	switch( g_selectModule )
	{
	case KIHX:
		RenderToBuffer();
		break;
	case WOOCOM:
		RenderToBuffer();
		break;
	case XTOZERO:
		RenderToBuffer();
		break;
	case COOLD:
		ClearBuffer();
		break;
	}
	
	// Initializes a screen image.
	//memset( g_pppScreenImage, 0, sizeof( g_pppScreenImage) );

	/*for ( int i = 0; i < SCREEN_HEIGHT; i++) 
	{
		for ( int j = 0; j < SCREEN_WIDTH; j++)
		{
			g_pppScreenImage[i][j][0] = (ubyte) 0;
			g_pppScreenImage[i][j][1] = (ubyte) 0;
			g_pppScreenImage[i][j][2] = (ubyte) 0;
		}
	}*/
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
	glPixelZoom( g_dZoomFactor, g_dZoomFactor );
	glCopyPixels( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_COLOR );
	glPixelZoom( 1.0, 1.0 );
	glFlush();
}

void keyboard( unsigned char key, int x, int y)
{
	printf( "[keyboard] key: %c\n", key );
	
	HMODULE hCurrentModule = NULL;

	switch( key )
	{
	case '1':
		LoadModuleKihx();
		break;

	case '2':
		InstallFunctionLoadMeshFromFile( WLoadMesh );
		InstallFunctionRenderToBuffer( WRender );
		printf( "\n<woocom>\n\n");
		g_selectModule = WOOCOM;
		break;

	case '3':
		LoadModuleXTZ();
		break;

	case '4':
		{	
			FixLater( Begin Explicit... )
			char* szDllName = "coold.dll";
			hCurrentModule = GetModuleHandle( szDllName );

			if( hCurrentModule == NULL)
			{
				hCurrentModule = LoadLibrary( szDllName );			
			}

			FnClearColorBuffer controlFunc = (FnClearColorBuffer)GetProcAddress( hCurrentModule, "colorControl" );
			if( controlFunc != nullptr )
			{				
				InstallFunctionClearColorBuffer(controlFunc);
			}			
			printf( "\n<coold>\n\n" );
			g_selectModule = COOLD;
		}
		break;

	case 'r':
	case 'R':
		g_dZoomFactor = 1.0;
		glutPostRedisplay();
		printf( "g_dZoomFactor reset to 1.0\n" );
		break;

	case 'z':
		g_dZoomFactor += 0.5;
		if ( g_dZoomFactor >= 3.0) 
		{
			g_dZoomFactor = 3.0;
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

