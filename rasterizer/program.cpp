// Include Directive
// 
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "glut.h"



// Constants Directive
//
#define	SCREEN_WIDTH 640
#define	SCREEN_HEIGHT 480
#define COLOR_DEPTH 3


// Global variables
//
typedef unsigned char ubyte;
ubyte g_pppScreenImage[SCREEN_HEIGHT][SCREEN_WIDTH][COLOR_DEPTH];


// Static variables
//
static GLdouble g_dZoomFactor = 1.0;
static GLint g_iHeight;


//-----------------------------------------------------------------------------------------------------------------------
//
// TODO THIS...
//
//-----------------------------------------------------------------------------------------------------------------------
void makeCheckImage(void)
{	
	// Initializes a screen image.
	memset(g_pppScreenImage, 0, sizeof(g_pppScreenImage));

	/*for (i = 0; i < SCREEN_HEIGHT; i++) 
	{
		for (j = 0; j < SCREEN_WIDTH; j++)
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
void init(void)
{    
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	makeCheckImage();
	glRasterPos2i(0, 480);
	glPixelZoom (1.f, -1.f);
	glDrawPixels(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, 
		GL_UNSIGNED_BYTE, g_pppScreenImage);
	glFlush();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	g_iHeight = (GLint) h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble) w, 0.0, (GLdouble) h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void motion(int x, int y)
{
	static GLint screeny;

	screeny = g_iHeight - (GLint) y;
	glRasterPos2i (x, screeny);
	glPixelZoom (g_dZoomFactor, g_dZoomFactor);
	glCopyPixels (0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_COLOR);
	glPixelZoom (1.0, 1.0);
	glFlush ();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	  case 'r':
	  case 'R':
		  g_dZoomFactor = 1.0;
		  glutPostRedisplay();
		  printf ("g_dZoomFactor reset to 1.0\n");
		  break;
	  case 'z':
		  g_dZoomFactor += 0.5;
		  if (g_dZoomFactor >= 3.0) 
			  g_dZoomFactor = 3.0;
		  printf ("g_dZoomFactor is now %4.1f\n", g_dZoomFactor);
		  break;
	  case 'Z':
		  g_dZoomFactor -= 0.5;
		  if (g_dZoomFactor <= 0.5) 
			  g_dZoomFactor = 0.5;
		  printf ("g_dZoomFactor is now %4.1f\n", g_dZoomFactor);
		  break;
	  case 27:
		  exit(0);
		  break;
	  default:
		  break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMotionFunc(motion);
	glutMainLoop();
	return 0; 
}

