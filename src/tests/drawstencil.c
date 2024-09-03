/**
 * Draw stencil region
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "glad/gl.h"
#include "glut_wrap.h"

static int Win;
static int WinWidth = 600, WinHeight = 200;
static GLfloat Xrot = 0.0, Yrot = 0.0;

#define WIDTH 256
#define HEIGHT 150

static GLubyte Image[HEIGHT][WIDTH];


static void
Draw(void)
{
   GLint x0 = 5, y0= 5, x1 = 10 + WIDTH, y1 = 5;
   GLubyte tmp[HEIGHT][WIDTH];
   GLint max, i, j;

   glClearColor(0.2, 0.2, 0.8, 0.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   glWindowPos2i(x0, y0);
   glDrawPixels(WIDTH, HEIGHT, GL_STENCIL_INDEX,
                GL_UNSIGNED_BYTE, (GLubyte*) Image);

   glReadPixels(x0, y0, WIDTH, HEIGHT, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, tmp);

   max = 0;
   for (i = 0; i < HEIGHT; i++) {
      for (j = 0; j < WIDTH; j++) {
         if (tmp[i][j] > max)
            max = tmp[i][j];
      }
   }
   printf("max = %d\n", max);

   glWindowPos2i(x1, y1);
   glDrawPixels(WIDTH, HEIGHT, GL_LUMINANCE, GL_UNSIGNED_BYTE, tmp);

   glutSwapBuffers();
}


static void
Reshape(int width, int height)
{
   WinWidth = width;
   WinHeight = height;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, width, 0, height, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}


static void
Key(unsigned char key, int x, int y)
{
   (void) x;
   (void) y;
   switch (key) {
   case 27:
      glutDestroyWindow(Win);
      exit(0);
      break;
   }
   glutPostRedisplay();
}


static void
SpecialKey(int key, int x, int y)
{
   const GLfloat step = 3.0;
   (void) x;
   (void) y;
   switch (key) {
   case GLUT_KEY_UP:
      Xrot -= step;
      break;
   case GLUT_KEY_DOWN:
      Xrot += step;
      break;
   case GLUT_KEY_LEFT:
      Yrot -= step;
      break;
   case GLUT_KEY_RIGHT:
      Yrot += step;
      break;
   }
   glutPostRedisplay();
}


static void
Init(void)
{
   int i, j;
   for (i = 0; i < HEIGHT; i++) {
      for (j = 0; j < WIDTH; j++) {
         Image[i][j] = j;
      }
   }
}


int
main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowSize(WinWidth, WinHeight);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_STENCIL);
   Win = glutCreateWindow(argv[0]);
   gladLoaderLoadGL();
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutSpecialFunc(SpecialKey);
   glutDisplayFunc(Draw);
   Init();
   glutMainLoop();
   gladLoaderUnloadGL();
   return 0;
}
