// Include Directive
// 
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

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

	static FnLoadMeshFromFile g_FnLoadMeshFromFile = NULL;
	static FnRenderToBuffer g_FnRenderToBuffer = NULL;


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

	inline void InstallFunctionLoadMeshFromFile( FnLoadMeshFromFile fp )
	{
		g_FnLoadMeshFromFile = fp;
		LoadMeshFromFile( "input.msh" );
	}

	inline void InstallFunctionRenderToBuffer( FnRenderToBuffer fp )
	{
		g_FnRenderToBuffer = fp;
		glutPostRedisplay();
	}
};


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
		Clear(255,255, 0);
		break;
	case XTOZERO:
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

	switch( key)
	{
		// kihx implementation
	case '1':
		InstallFunctionLoadMeshFromFile( kiLoadMeshFromFile );
		InstallFunctionRenderToBuffer( kiRenderToBuffer );
		printf( "\n<kihx>\n\n" );
		g_selectModule = KIHX;
		break;
	case '2':
		Initialize( g_pppScreenImage, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_DEPTH);
		glutPostRedisplay();
		printf( "\n<woocom>\n\n");
		g_selectModule = WOOCOM;
		break;
	case '3':
		ClearBuffer( g_pppScreenImage, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_DEPTH);
		glutPostRedisplay();
		printf( "\n<xtozero>\n\n");
		g_selectModule = XTOZERO;
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

