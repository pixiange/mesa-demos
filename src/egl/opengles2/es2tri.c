/*
 * Copyright (C) 2008  VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

/*
 * Draw a triangle with X/EGL and OpenGL ES 2.x
 */

#define USE_FULL_GL 0



#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if USE_FULL_GL
#include "gl_wrap.h"  /* use full OpenGL */
#else
#include <GLES2/gl2.h>  /* use OpenGL ES 2.x */
#endif
#include <EGL/egl.h>
#include "eglut.h"

#include "matrix.h"

#define FLOAT_TO_FIXED(X)   ((X) * 65535.0)



static GLfloat view_rotx = 0.0, view_roty = 0.0;

static GLint u_matrix = -1;
static GLint attr_pos = 0, attr_color = 1;

static void
draw(void)
{
   static const GLfloat verts[3][2] = {
      { -1, -1 },
      {  1, -1 },
      {  0,  1 }
   };
   static const GLfloat colors[3][3] = {
      { 1, 0, 0 },
      { 0, 1, 0 },
      { 0, 0, 1 }
   };
   float mat[4][4];

   /* Set modelview/projection matrix */
   mat4_identity(mat);
   mat4_rotate(mat, view_rotx * (M_PI / 180.0), 1, 0, 0);
   mat4_rotate(mat, view_roty * (M_PI / 180.0), 0, 1, 0);
   mat4_scale(mat, 0.5, 0.5, 0.5);
   glUniformMatrix4fv(u_matrix, 1, GL_FALSE, (GLfloat *)mat);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   {
      glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
      glVertexAttribPointer(attr_color, 3, GL_FLOAT, GL_FALSE, 0, colors);
      glEnableVertexAttribArray(attr_pos);
      glEnableVertexAttribArray(attr_color);

      glDrawArrays(GL_TRIANGLES, 0, 3);

      glDisableVertexAttribArray(attr_pos);
      glDisableVertexAttribArray(attr_color);
   }
}


/* new window size or exposure */
static void
reshape(int width, int height)
{
   glViewport(0, 0, (GLint) width, (GLint) height);
   eglutPostRedisplay();
}


static void
create_shaders(void)
{
   static const char *fragShaderText =
      "precision mediump float;\n"
      "varying vec4 v_color;\n"
      "void main() {\n"
      "   gl_FragColor = v_color;\n"
      "}\n";
   static const char *vertShaderText =
      "uniform mat4 modelviewProjection;\n"
      "attribute vec4 pos;\n"
      "attribute vec4 color;\n"
      "varying vec4 v_color;\n"
      "void main() {\n"
      "   gl_Position = modelviewProjection * pos;\n"
      "   v_color = color;\n"
      "}\n";

   GLuint fragShader, vertShader, program;
   GLint stat;

   fragShader = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fragShader, 1, (const char **) &fragShaderText, NULL);
   glCompileShader(fragShader);
   glGetShaderiv(fragShader, GL_COMPILE_STATUS, &stat);
   if (!stat) {
      printf("Error: fragment shader did not compile!\n");
      exit(1);
   }

   vertShader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vertShader, 1, (const char **) &vertShaderText, NULL);
   glCompileShader(vertShader);
   glGetShaderiv(vertShader, GL_COMPILE_STATUS, &stat);
   if (!stat) {
      printf("Error: vertex shader did not compile!\n");
      exit(1);
   }

   program = glCreateProgram();
   glAttachShader(program, fragShader);
   glAttachShader(program, vertShader);
   glLinkProgram(program);

   glGetProgramiv(program, GL_LINK_STATUS, &stat);
   if (!stat) {
      char log[1000];
      GLsizei len;
      glGetProgramInfoLog(program, 1000, &len, log);
      printf("Error: linking:\n%s\n", log);
      exit(1);
   }

   glUseProgram(program);

   if (1) {
      /* test setting attrib locations */
      glBindAttribLocation(program, attr_pos, "pos");
      glBindAttribLocation(program, attr_color, "color");
      glLinkProgram(program);  /* needed to put attribs into effect */
   }
   else {
      /* test automatic attrib locations */
      attr_pos = glGetAttribLocation(program, "pos");
      attr_color = glGetAttribLocation(program, "color");
   }

   u_matrix = glGetUniformLocation(program, "modelviewProjection");
   printf("Uniform modelviewProjection at %d\n", u_matrix);
   printf("Attrib pos at %d\n", attr_pos);
   printf("Attrib color at %d\n", attr_color);
}


static void
init(void)
{
   glClearColor(0.4, 0.4, 0.4, 0.0);

   create_shaders();
}


static void
special(int key)
{
      switch (key) {
      case EGLUT_KEY_LEFT:
         view_roty += 5.0;
         break;
      case EGLUT_KEY_RIGHT:
         view_roty -= 5.0;
         break;
      case EGLUT_KEY_UP:
         view_rotx += 5.0;
         break;
      case EGLUT_KEY_DOWN:
         view_rotx -= 5.0;
         break;
   }
   eglutPostRedisplay();
}


static void
usage(void)
{
   printf("Usage:\n");
   printf("  -display <displayname>  set the display to run on\n");
   printf("  -info                   display OpenGL renderer info\n");
}


int
main(int argc, char *argv[])
{
   GLboolean printInfo = GL_FALSE;

   eglutInitWindowSize(300, 300);
   eglutInitAPIMask(EGLUT_OPENGL_ES2_BIT);
   eglutInit(argc, argv);

   eglutCreateWindow("OpenGL ES 2.x tri");

   for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-display") == 0) {
         /* eglutInit() handled this; consume the next arg and move on */
         i++;
      }
      else if (strcmp(argv[i], "-info") == 0) {
         printInfo = GL_TRUE;
      }
      else {
         usage();
         return -1;
      }
   }

   if (printInfo) {
      printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
      printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
      printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
      printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
   }

   init();

   eglutReshapeFunc(reshape);
   eglutDisplayFunc(draw);
   eglutSpecialFunc(special);

   eglutMainLoop();

   return 0;
}
