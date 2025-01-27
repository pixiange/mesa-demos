/*
 * Mesa 3-D graphics library
 * Version:  7.9
 *
 * Copyright (C) 2010 LunarG Inc.
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
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 *    Alexandros Frantzis <alexandros.frantzis@linaro.org> (GLES2.0 port)
 */

/*
 * This demo uses EGL_KHR_image_pixmap and GL_OES_EGL_image to demonstrate
 * texture-from-pixmap.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> /* for usleep */
#include <sys/time.h> /* for gettimeofday */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <math.h>
#include "matrix.h"

#define DEG2RAD(d) (2 * M_PI * (d) / 360.0)

static const char vertex_shader[] =
"#ifdef GL_ES\n"
"precision mediump float;\n"
"#endif\n"
"attribute vec3 position;\n"
"attribute vec2 texcoord;\n"
"\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"\n"
"varying vec2 TextureCoord;\n"
"\n"
"void main(void)\n"
"{\n"
"    TextureCoord = texcoord;\n"
"    // Transform the position to clip coordinates\n"
"    gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);\n"
"}";

static const char fragment_shader[] =
"#ifdef GL_ES\n"
"precision mediump float;\n"
"#endif\n"
"uniform sampler2D Texture0;\n"
"varying vec2 TextureCoord;\n"
"\n"
"void main(void)\n"
"{\n"
"    vec4 texel = texture2D(Texture0, TextureCoord);\n"
"    gl_FragColor = texel;\n"
"}";

struct app_data {
   /* native */
   Display *xdpy;
   Window canvas, cube;
   Pixmap pix;
   unsigned int width, height, depth;
   GC fg, bg;
   XImage *ximage;

   /* EGL */
   EGLDisplay dpy;
   EGLContext ctx;
   EGLSurface surf;
   EGLImageKHR img;
   PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
   PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;

   /* OpenGL ES */
   GLuint texture;
   PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;

   /* Shaders */
   float projection_matrix[4][4];
   float model_view_matrix[4][4];
   GLuint model_view_projection_matrix_loc;
   GLuint texture0_loc;

   /* app state */
   Bool loop;
   Bool redraw, reshape;

   struct {
      Bool active;
      unsigned long next_frame; /* in ms */
      float view_rotx;
      float view_roty;
      float view_rotz;

   } animate;

   struct {
      Bool active;
      int x1, y1;
      int x2, y2;
   } paint;

   struct {
      Bool force_pot;
      Bool use_egl_image;
   } options;
};

static unsigned int
nearest_pot(unsigned int n)
{
    int k;
    if (n == 0)
        return 1;
    for (k = sizeof(unsigned int) * 8 - 1; ((1U << k) & n) == 0; k--);
    if (((1U << (k - 1)) & n) == 0)
        return (1U) << k;
    return (1U) << (k + 1);
}

static void
gl_redraw(struct app_data *data)
{
   const GLfloat verts[4][3] = {
      { -1, -1, 0 },
      {  1, -1, 0 },
      {  1,  1, 0 },
      { -1,  1, 0 }
   };
   const GLfloat texcoords[4][2] = {
      { 0, 1 },
      { 1, 1 },
      { 1, 0 },
      { 0, 0 }
   };
   const GLfloat faces[6][4] = {
      {   0, 0, 1, 0 },
      {  90, 0, 1, 0 },
      { 180, 0, 1, 0 },
      { 270, 0, 1, 0 },
      {  90, 1, 0, 0 },
      { -90, 1, 0, 0 }
   };
   GLint i;

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, data->texture);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* Set up the position of the attributes in the vertex array */
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verts);
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texcoords);

   /* Enable the attributes */
   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);

   float transform[4][4];
   float model_view[4][4];
   float model_view_projection[4][4];

   /* Rotate the view */
   memcpy(transform, data->model_view_matrix, sizeof(transform));
   mat4_rotate(transform, DEG2RAD(data->animate.view_rotx), 1, 0, 0);
   mat4_rotate(transform, DEG2RAD(data->animate.view_roty), 0, 1, 0);
   mat4_rotate(transform, DEG2RAD(data->animate.view_rotz), 0, 0, 1);

   /* Draw the triangle strips that comprise the cube */
   for (i = 0; i < 6; i++) {
      /* Set up transformation for each face of the cube */
      memcpy(model_view, transform, sizeof(model_view));
      mat4_rotate(model_view, DEG2RAD(faces[i][0]), faces[i][1],
                    faces[i][2], faces[i][3]);
      mat4_translate(model_view, 0.0, 0.0, 1.0);
      memcpy(model_view_projection, data->projection_matrix,
             sizeof(model_view_projection));
      mat4_multiply(model_view_projection, model_view);

      /* Load shader ModelViewProjection uniform */
      glUniformMatrix4fv(data->model_view_projection_matrix_loc, 1, GL_FALSE,
                         (GLfloat *)model_view_projection);

      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
   }

   /* Disable the attributes */
   glDisableVertexAttribArray(1);
   glDisableVertexAttribArray(0);
}

static void
gl_reshape(struct app_data *data)
{
   GLfloat ar = data->width / (float)data->height;

   /* Update the projection matrix */
   mat4_perspective_gl(data->projection_matrix, 22.6, ar, 5.0, 60.0);

   /* Update the modelview matrix */
   mat4_identity(data->model_view_matrix);
   mat4_translate(data->model_view_matrix, 0.0, 0.0, -10.0);

   /* Set the viewport */
   glViewport(0, 0, (GLint) data->width, (GLint) data->height);
}

static void
app_redraw(struct app_data *data)
{
   /* if the pixmap has changed... */
   if (data->reshape || data->paint.active) {
      eglWaitNative(EGL_CORE_NATIVE_ENGINE);

      /*
       * The extension only states that
       *
       * If an application specifies an EGLImage sibling as the destination
       * for rendering and/or pixel download operations (e.g., as an
       * OpenGL-ES framebuffer object, glTexSubImage2D, etc.), the modified
       * image results will be observed by all EGLImage siblings in all
       * client API contexts.
       *
       * Though not required by the drivers I tested, I think the rules of
       * "Propagating Changes to Objects" should apply here. That is, the
       * changes made by the native engine must be completed and the resource
       * must be re-attached.
       */
      if (data->options.use_egl_image) {
         data->glEGLImageTargetTexture2DOES(GL_TEXTURE_2D,
                  (GLeglImageOES) data->img);
      }
      else {
         /* Normal texture data upload */

         /*
          * It seems that is faster to call XGetImage rather than
          * XGetSubImage...
          */
         if (data->ximage)
            XDestroyImage(data->ximage);
         data->ximage = XGetImage(data->xdpy, data->pix, 0, 0,
                                  data->width, data->height,
                                  AllPlanes, ZPixmap);

         /*
          * Note that the data we get from the pixmap is BGRA but the texture
          * expects RGBA, so what we are doing here is technically incorrect.
          * However, for the black-and-white pixmaps we care about here it
          * doesn't make any difference.
          */
         glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                         data->width, data->height,
                         GL_RGBA, GL_UNSIGNED_BYTE,
                         data->ximage->data);
      }

      /* Update the on-screen representation of the pixmap */
      XCopyArea(data->xdpy, data->pix, data->canvas, data->fg,
                0, 0, data->width, data->height, 0, 0);
   }

   gl_redraw(data);

   eglSwapBuffers(data->dpy, data->surf);
}

static void
app_reshape(struct app_data *data)
{
   const EGLint img_attribs[] = {
      EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
      EGL_NONE
   };

   XResizeWindow(data->xdpy, data->cube, data->width, data->height);
   XMoveWindow(data->xdpy, data->cube, data->width, 0);

   if (data->img)
      data->eglDestroyImageKHR(data->dpy, data->img);
   if (data->pix)
      XFreePixmap(data->xdpy, data->pix);

   data->pix = XCreatePixmap(data->xdpy, data->canvas, data->width,
                             data->height, data->depth);
   XFillRectangle(data->xdpy, data->pix, data->bg, 0, 0, data->width,
                  data->height);

   if (data->options.use_egl_image) {
      data->img = data->eglCreateImageKHR(data->dpy, EGL_NO_CONTEXT,
            EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer) data->pix, img_attribs);
   }
   else {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                   data->width, data->height, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   }

   gl_reshape(data);
}

static void
app_toggle_animate(struct app_data *data)
{
   data->animate.active = !data->animate.active;

   if (data->animate.active) {
      struct timeval tv;

      gettimeofday(&tv, NULL);
      data->animate.next_frame = tv.tv_sec * 1000 + tv.tv_usec / 1000;
   }
}

static void
app_next_event(struct app_data *data)
{
   XEvent event;

   data->reshape = False;
   data->redraw = False;
   data->paint.active = False;

   if (data->animate.active) {
      struct timeval tv;
      unsigned long now;

      gettimeofday(&tv, NULL);
      now = tv.tv_sec * 1000 + tv.tv_usec / 1000;

      /* wait for next frame */
      if (!XPending(data->xdpy) && now < data->animate.next_frame) {
         usleep((data->animate.next_frame - now) * 1000);
         gettimeofday(&tv, NULL);
         now = tv.tv_sec * 1000 + tv.tv_usec / 1000;
      }

      while (now >= data->animate.next_frame) {
         data->animate.view_rotx += 1.0;
         data->animate.view_roty += 2.0;
         data->animate.view_rotz += 1.5;

         /* 30fps */
         data->animate.next_frame += 1000 / 30;
      }

      /* check again in case there were events when sleeping */
      if (!XPending(data->xdpy)) {
         data->redraw = True;
         return;
      }
   }

   XNextEvent(data->xdpy, &event);

   switch (event.type) {
   case ConfigureNotify:
      data->width = event.xconfigure.width / 2;
      data->height = event.xconfigure.height;
      data->reshape = True;
      if (data->options.force_pot) {
          data->width = nearest_pot(data->width);
          data->height = nearest_pot(data->height);
      }
      break;
   case Expose:
      data->redraw = True;
      break;
   case KeyPress:
      {
         int code;

         code = XLookupKeysym(&event.xkey, 0);
         switch (code) {
         case XK_a:
            app_toggle_animate(data);
            break;
         case XK_Escape:
            data->loop = False;
            break;
         default:
            break;
         }
      }
      break;
   case ButtonPress:
      data->paint.x1 = data->paint.x2 = event.xbutton.x;
      data->paint.y1 = data->paint.y2 = event.xbutton.y;
      break;
   case ButtonRelease:
      data->paint.active = False;
      break;
   case MotionNotify:
      data->paint.x1 = data->paint.x2;
      data->paint.y1 = data->paint.y2;
      data->paint.x2 = event.xmotion.x;
      data->paint.y2 = event.xmotion.y;
      data->paint.active = True;
      break;
   default:
      break;
   }

   if (data->paint.active || data->reshape)
      data->redraw = True;
}

static void
app_init_gl(struct app_data *data)
{
   GLuint v, f, program;
   const char *p;
   char msg[512];

   glClearColor(0.1, 0.1, 0.3, 0.0);

   glGenTextures(1, &data->texture);

   glBindTexture(GL_TEXTURE_2D, data->texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);

   /* Compile the vertex shader */
   p = vertex_shader;
   v = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(v, 1, &p, NULL);
   glCompileShader(v);
   glGetShaderInfoLog(v, sizeof msg, NULL, msg);
   printf("vertex shader compilation info: %s\n", msg);

   /* Compile the fragment shader */
   p = fragment_shader;
   f = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(f, 1, &p, NULL);
   glCompileShader(f);
   glGetShaderInfoLog(f, sizeof msg, NULL, msg);
   printf("fragment shader compilation info: %s\n", msg);

   /* Create and link the shader program */
   program = glCreateProgram();
   glAttachShader(program, v);
   glAttachShader(program, f);
   glBindAttribLocation(program, 0, "position");
   glBindAttribLocation(program, 1, "texcoord");

   glLinkProgram(program);
   glGetProgramInfoLog(program, sizeof msg, NULL, msg);
   printf("shader program link info: %s\n", msg);

   /* Enable the shaders */
   glUseProgram(program);

   /* Get the locations of the uniforms so we can access them */
   data->model_view_projection_matrix_loc =
         glGetUniformLocation(program, "ModelViewProjectionMatrix");
   data->texture0_loc = glGetUniformLocation(program, "Texture0");

   /* We just use the first texture unit throughout the program */
   glUniform1i(data->texture0_loc, 0);
}

static Bool
app_init_exts(struct app_data *data)
{
   const char *exts;
   Bool result = True;

   exts = eglQueryString(data->dpy, EGL_EXTENSIONS);
   printf("==== EGL extensions ====\n%s\n", exts);
   data->eglCreateImageKHR =
      (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
   data->eglDestroyImageKHR =
      (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");
   if (!exts || !strstr(exts, "EGL_KHR_image_pixmap") ||
       !data->eglCreateImageKHR || !data->eglDestroyImageKHR) {
      printf("** EGL does not support EGL_KHR_image_pixmap!\n");
      result &= False;
   }

   exts = (const char *) glGetString(GL_EXTENSIONS);
   printf("==== GL extensions ====\n%s\n", exts);
   data->glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)
      eglGetProcAddress("glEGLImageTargetTexture2DOES");
   if (!exts || !strstr(exts, "GL_OES_EGL_image") ||
       !data->glEGLImageTargetTexture2DOES) {
      printf("** OpenGL ES2.0 does not support GL_OES_EGL_image!\n");
      result &= False;
   }
   printf("=======================\n");

   return result;
}

static void
app_run(struct app_data *data)
{
   Window root;
   int x, y;
   unsigned int border;

   if (!eglMakeCurrent(data->dpy, data->surf, data->surf, data->ctx))
      return;

   if (!app_init_exts(data)) {
      data->options.use_egl_image = False;
      printf("** Required EGL/GLES2.0 extensions not present.\n"
             "** Falling back to XImage/glTexImage2D texture update method!\n");
   }

   app_init_gl(data);

   printf("==> Draw something on the left with the mouse! <==\n");

   if (!XGetGeometry(data->xdpy, data->canvas, &root,
                     &x, &y, &data->width, &data->height,
                     &border, &data->depth))
      return;
   data->width /= 2;
   if (data->options.force_pot) {
       data->width = nearest_pot(data->width);
       data->height = nearest_pot(data->height);
   }

   /* Force an initial resize/redraw */
   data->paint.active = True;
   data->reshape = True;
   app_reshape(data);
   app_redraw(data);

   XMapWindow(data->xdpy, data->canvas);
   XMapWindow(data->xdpy, data->cube);

   app_toggle_animate(data);
   data->loop = True;

   while (data->loop) {
      app_next_event(data);

      if (data->reshape)
         app_reshape(data);
      if (data->paint.active) {
         XDrawLine(data->xdpy, data->pix, data->fg,
                   data->paint.x1, data->paint.y1,
                   data->paint.x2, data->paint.y2);
      }

      if (data->redraw)
         app_redraw(data);
   }

   eglMakeCurrent(data->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

static Bool
make_x_window(struct app_data *data, const char *name,
              int x, int y, int width, int height)
{
   static const EGLint attribs[] = {
      EGL_RED_SIZE, 0,
      EGL_GREEN_SIZE, 0,
      EGL_BLUE_SIZE, 0,
      EGL_DEPTH_SIZE, 0,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_NONE
   };
   static const EGLint ctx_attribs[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };
   int scrnum;
   XSetWindowAttributes attr;
   unsigned long mask;
   Window root;
   Window win;
   XVisualInfo *visInfo, visTemplate;
   int num_visuals;
   EGLConfig config;
   EGLint num_configs;
   EGLint vid;

   scrnum = DefaultScreen(data->xdpy);
   root = RootWindow(data->xdpy, scrnum);

   if (!eglChooseConfig(data->dpy, attribs, &config, 1, &num_configs)) {
      printf("Error: couldn't get an EGL visual config\n");
      exit(1);
   }

   assert(config);
   assert(num_configs > 0);

   if (!eglGetConfigAttrib(data->dpy, config, EGL_NATIVE_VISUAL_ID, &vid)) {
      printf("Error: eglGetConfigAttrib() failed\n");
      exit(1);
   }

   /* The X window visual must match the EGL config */
   visTemplate.visualid = vid;
   visInfo = XGetVisualInfo(data->xdpy, VisualIDMask, &visTemplate, &num_visuals);
   if (!visInfo) {
      printf("Error: couldn't get X visual\n");
      exit(1);
   }

   /* window attributes */
   attr.background_pixel = 0;
   attr.border_pixel = 0;
   attr.colormap = XCreateColormap(data->xdpy, root, visInfo->visual, AllocNone);
   attr.event_mask = StructureNotifyMask | ExposureMask |
                     KeyPressMask | ButtonPressMask | ButtonMotionMask;
   mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

   win = XCreateWindow(data->xdpy, root, 0, 0, width * 2, height,
                       0, visInfo->depth, InputOutput,
                       visInfo->visual, mask, &attr);

   /* set hints and properties */
   {
      XSizeHints sizehints;
      sizehints.x = x;
      sizehints.y = y;
      sizehints.width  = width;
      sizehints.height = height;
      sizehints.flags = USSize | USPosition;
      XSetNormalHints(data->xdpy, win, &sizehints);
      XSetStandardProperties(data->xdpy, win, name, name,
                              None, (char **)NULL, 0, &sizehints);
   }

   data->canvas = win;

   attr.event_mask = 0x0;
   win = XCreateWindow(data->xdpy, win, width, 0, width, height,
                       0, visInfo->depth, InputOutput,
                       visInfo->visual, mask, &attr);
   data->cube = win;

   eglBindAPI(EGL_OPENGL_ES_API);

   data->ctx = eglCreateContext(data->dpy, config, EGL_NO_CONTEXT, ctx_attribs);
   if (!data->ctx) {
      printf("Error: eglCreateContext failed\n");
      exit(1);
   }

   data->surf = eglCreateWindowSurface(data->dpy, config, data->cube, NULL);
   if (!data->surf) {
      printf("Error: eglCreateWindowSurface failed\n");
      exit(1);
   }

   XFree(visInfo);

   return (data->canvas && data->cube && data->ctx && data->surf);
}

static void
app_fini(struct app_data *data)
{
   if (data->img)
      data->eglDestroyImageKHR(data->dpy, data->img);
   if (data->pix)
      XFreePixmap(data->xdpy, data->pix);

   if (data->fg)
      XFreeGC(data->xdpy, data->fg);
   if (data->bg)
      XFreeGC(data->xdpy, data->bg);

   if (data->surf)
      eglDestroySurface(data->dpy, data->surf);
   if (data->ctx)
      eglDestroyContext(data->dpy, data->ctx);

   if (data->cube)
      XDestroyWindow(data->xdpy, data->cube);
   if (data->canvas)
      XDestroyWindow(data->xdpy, data->canvas);

   if (data->dpy)
      eglTerminate(data->dpy);
   if (data->xdpy)
      XCloseDisplay(data->xdpy);
}

static Bool
parse_args(struct app_data *data, int argc, char **argv)
{
   int i;
   data->options.force_pot = False;
   data->options.use_egl_image = True;

   for (i = 1; i < argc; i++) {
       if (!strcmp(argv[i], "--force-pot"))
          data->options.force_pot = True;
       else if (!strcmp(argv[i], "--no-egl-image"))
          data->options.use_egl_image = False;
       else if (!strcmp(argv[i], "--help")) {
          printf("An EGL/GLES2.0 example of using EGL_KHR_image_pixmap for texture-from-pixmap.\n"
                 "\n"
                 "Options:\n"
                 "  --force-pot    Force the usage of power of two textures\n"
                 "  --no-egl-image Don't use EGLImage for accelerated TFP (use glTexImage2D)\n"
                 "  --help         Display help\n");
          return False;
       }
   }

   return True;
}

static Bool
app_init(struct app_data *data, int argc, char **argv)
{
   XGCValues gc_vals;

   memset(data, 0, sizeof(*data));

   if (!parse_args(data, argc, argv))
      return False;

   data->xdpy = XOpenDisplay(NULL);
   if (!data->xdpy)
      goto fail;

   data->dpy = eglGetDisplay(data->xdpy);
   if (!data->dpy || !eglInitialize(data->dpy, NULL, NULL))
      goto fail;

   if (!make_x_window(data, "EGLImage TFP", 0, 0, 300, 300))
      goto fail;

   gc_vals.function = GXcopy;
   gc_vals.foreground = WhitePixel(data->xdpy, DefaultScreen(data->xdpy));
   gc_vals.line_width = 3;
   gc_vals.line_style = LineSolid;
   gc_vals.fill_style = FillSolid;

   data->fg = XCreateGC(data->xdpy, data->canvas,
         GCFunction | GCForeground | GCLineWidth | GCLineStyle | GCFillStyle,
         &gc_vals);
   gc_vals.foreground = BlackPixel(data->xdpy, DefaultScreen(data->xdpy));
   data->bg = XCreateGC(data->xdpy, data->canvas,
         GCFunction | GCForeground | GCLineWidth | GCLineStyle | GCFillStyle,
         &gc_vals);
   if (!data->fg || !data->bg)
      goto fail;

   return True;

fail:
   app_fini(data);
   return False;
}

int
main(int argc, char **argv)
{
   struct app_data data;

   if (app_init(&data, argc, argv)) {
      app_run(&data);
      app_fini(&data);
   }

   return 0;
}
