/*
 * eglinfo - like glxinfo but for EGL
 *
 * Brian Paul
 * 11 March 2005
 *
 * Copyright (C) 2005  Brian Paul   All Rights Reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#define EGL_PLATFORM_ANGLE_ANGLE 0x3202

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glinfo_common.h"
#include "glad/egl.h"
#include "glad/gl.h"

#define MAX_CONFIGS 1000
#define MAX_MODES 1000
#define MAX_SCREENS 10
#define MAX_COLUMN 70

#define ISALPHANUM(A) ((A>='0' && A<='9') || (A>='A' && A<='Z'))


/* These are X visual types, so if you're running eglinfo under
 * something not X, they probably don't make sense. */
static const char *vnames[] = { "SG", "GS", "SC", "PC", "TC", "DC" };

struct platform {
   /* names as defined in the EGL spec */
   const char *names[2];
   /* used by parse_args */
   const char *short_name;
   /* a human-readable name */
   const char *human_name;
   EGLenum platform_enum;
};

enum api {
   OPENGL = 0,
   OPENGL_CORE = 1,
   OPENGL_ES = 2,
   /* OpenVG = 3, */
   ALL = ~0,
};

static const char *apis[3] = {
   [OPENGL] = "gl",
   [OPENGL_CORE] = "glcore",
   [OPENGL_ES] = "gles",
   /* [OpenVG] = "vg", */
};

static const struct platform platforms[] = {
   {
      .names = { "EGL_KHR_platform_android" },
      .short_name = "android",
      .human_name = "Android",
      .platform_enum = EGL_PLATFORM_ANDROID_KHR,
   },
   {
      .names = { "EGL_MESA_platform_gbm", "EGL_KHR_platform_gbm" },
      .short_name = "gbm",
      .human_name = "GBM",
      .platform_enum = EGL_PLATFORM_GBM_KHR,
   },
   {
      .names = { "EGL_EXT_platform_wayland", "EGL_KHR_platform_wayland" },
      .short_name = "wayland",
      .human_name = "Wayland",
      .platform_enum = EGL_PLATFORM_WAYLAND_KHR,
   },
   {
      .names = { "EGL_EXT_platform_x11", "EGL_KHR_platform_x11" },
      .short_name = "x11",
      .human_name = "X11",
      .platform_enum = EGL_PLATFORM_X11_KHR,
   },
   {
      .names = { "EGL_MESA_platform_surfaceless" },
      .short_name = "surfaceless",
      .human_name = "Surfaceless",
      .platform_enum = EGL_PLATFORM_SURFACELESS_MESA,
   },
   {
      .names = { "EGL_ANGLE_platform_angle" },
      .short_name = "angle",
      .human_name = "ANGLE",
      .platform_enum = EGL_PLATFORM_ANGLE_ANGLE,
   },
};

struct eglconfig_info {
   EGLint id;
   EGLint size;
   EGLint level;

   EGLint red;
   EGLint green;
   EGLint blue;
   EGLint alpha;

   EGLint depth;
   EGLint stencil;

   EGLint renderable;
   EGLint surfaces;
   EGLint vid;
   EGLint vtype;
   EGLint caveat;
   EGLint bind_rgb;
   EGLint bind_rgba;

   EGLint samples;
   EGLint sample_buffers;
};

struct options {
   unsigned platform;
   unsigned api;
   InfoMode mode;
   EGLBoolean single_line;
   EGLBoolean limits;
};

/**
 * Print a concise table of all available configurations.
 */
static void
PrintConfigsNormal(unsigned num_configs, struct eglconfig_info *info)
{
   printf("Configurations:\n");
   printf("           bf lv colorbuffer dp st  ms    vis   cav bi  renderable  supported\n");
   printf(" id (hex)  sz  l  r  g  b  a th cl ns b    id   eat nd gl es es2 vg surfaces \n");
   /*        ^        ^   ^  ^  ^  ^  ^ ^  ^  ^  ^    ^    ^   ^  ^  ^  ^   ^  ^
    *        |        |   |  |  |  |  | |  |  |  |    |    |   |  |  |  |   |  |
    *        |        |   |  |  |  |  | |  |  |  |    |    |   |  |  |  |   |  EGL_SURFACE_TYPE
    *        |        |   |  |  |  |  | |  |  |  |    |    |   |  EGL_RENDERABLE_TYPE
    *        |        |   |  |  |  |  | |  |  |  |    |    |   EGL_BIND_TO_TEXTURE_RGB/EGL_BIND_TO_TEXTURE_RGBA
    *        |        |   |  |  |  |  | |  |  |  |    |    EGL_CONFIG_CAVEAT
    *        |        |   |  |  |  |  | |  |  |  |    EGL_NATIVE_VISUAL_ID/EGL_NATIVE_VISUAL_TYPE
    *        |        |   |  |  |  |  | |  |  |  EGL_SAMPLE_BUFFERS
    *        |        |   |  |  |  |  | |  |  EGL_SAMPLES
    *        |        |   |  |  |  |  | |  EGL_STENCIL_SIZE
    *        |        |   |  |  |  |  | EGL_DEPTH_SIZE
    *        |        |   |  |  |  |  EGL_ALPHA_SIZE
    *        |        |   |  |  |  EGL_BLUE_SIZE
    *        |        |   |  |  EGL_GREEN_SIZE
    *        |        |   |  EGL_RED_SIZE
    *        |        |   EGL_LEVEL
    *        |        EGL_BUFFER_SIZE
    *        EGL_CONFIG_ID
    */
   printf("------------------------------------------------------------------------------\n");
   for (int i = 0; i < num_configs; i++) {
      char surface[100] = "";

      if (info[i].surfaces & EGL_WINDOW_BIT)
         strcat(surface, "win,");
      if (info[i].surfaces & EGL_PBUFFER_BIT)
         strcat(surface, "pb,");
      if (info[i].surfaces & EGL_PIXMAP_BIT)
         strcat(surface, "pix,");
      if (info[i].surfaces & EGL_STREAM_BIT_KHR)
         strcat(surface, "str,");
      if (strlen(surface) > 0)
         surface[strlen(surface) - 1] = 0;

      EGLint vid = info[i].vid;
      char vidstr[5] = {0,0,0,0,0};
      vidstr[0] = (vid>> 0) & 0xff;
      vidstr[1] = (vid>> 8) & 0xff;
      vidstr[2] = (vid>>16) & 0xff;
      vidstr[3] = (vid>>24) & 0xff;
      if (!ISALPHANUM(vidstr[0]) || !ISALPHANUM(vidstr[1]) || !ISALPHANUM(vidstr[2]) || !ISALPHANUM(vidstr[3]))
         snprintf(vidstr, sizeof(vidstr), "%04x", vid);

      printf("%3d (0x%02x) %2d %2d %2d %2d %2d %2d %2d %2d %2d%2d  %s%s ",
             info[i].id, info[i].id, info[i].size, info[i].level,
             info[i].red, info[i].green, info[i].blue, info[i].alpha,
             info[i].depth, info[i].stencil,
             info[i].samples, info[i].sample_buffers, vidstr,
             info[i].vtype < 6 ? vnames[info[i].vtype] : "--");
      printf("  %c  %c  %c  %c  %c   %c %s\n",
             (info[i].caveat != EGL_NONE) ? 'y' : ' ',
             (info[i].bind_rgba) ? 'a' : (info[i].bind_rgb) ? 'y' : ' ',
             (info[i].renderable & EGL_OPENGL_BIT) ? 'y' : ' ',
             (info[i].renderable & EGL_OPENGL_ES_BIT) ? 'y' : ' ',
             (info[i].renderable & EGL_OPENGL_ES2_BIT) ? 'y' : ' ',
             (info[i].renderable & EGL_OPENVG_BIT) ? 'y' : ' ',
             surface);
   }
}

/**
 * Print a verbose table of all available configurations.
 */
static void
PrintConfigsVerbose(unsigned num_configs, struct eglconfig_info *info)
{
   printf("Configurations (%u in total):\n", num_configs);

   for (int i = 0; i < num_configs; i++) {
      printf("\nEGL_CONFIG_ID: %d (%3x)", info[i].id, info[i].id);
      printf("    EGL_BUFFER_SIZE: %d", info[i].size);
      printf("    EGL_LEVEL: %d", info[i].level);

      printf("\n\tEGL_RED_SIZE: %d", info[i].red);
      printf("    EGL_GREEN_SIZE: %d", info[i].green);
      printf("    EGL_BLUE_SIZE: %d", info[i].blue);
      printf("    EGL_ALPHA_SIZE: %d", info[i].alpha);

      printf("\n\tEGL_DEPTH_SIZE: %d", info[i].depth);
      printf("    EGL_STENCIL_SIZE: %d", info[i].stencil);

      printf("\n\tEGL_SAMPLES: %d", info[i].samples);
      printf("    EGL_SAMPLE_BUFFERS: %d", info[i].sample_buffers);

      printf("\n\tEGL_NATIVE_VISUAL_ID: %x", info[i].vid);
      printf("    EGL_NATIVE_VISUAL_TYPE: %x", info[i].vtype);
      printf("    EGL_CONFIG_CAVEAT: %s",
             info[i].caveat == EGL_NONE ? "none" :
             info[i].caveat == EGL_SLOW_CONFIG ? "slow" : 
             info[i].caveat == EGL_NON_CONFORMANT_CONFIG ? "non-conformant" :
             "(unknown)");
      printf("\n\tEGL_BIND_TO_TEXTURE: %s", info[i].bind_rgb ? "rgb" :
                                            info[i].bind_rgba ? "rgba" :
                                            "no");

      char renderable[100] = "";

      if (info[i].renderable & EGL_OPENGL_BIT)
         strcat(renderable, "opengl,");
      if (info[i].renderable & EGL_OPENGL_ES_BIT)
         strcat(renderable, "opengles,");
      if (info[i].renderable & EGL_OPENGL_ES2_BIT)
         strcat(renderable, "opengles2,");
      if (info[i].renderable & EGL_OPENVG_BIT)
         strcat(renderable, "openvg,");
      /* remove the last comma */
      renderable[strlen(renderable) - 1] = '\0';

      printf("\n\tEGL_RENDERABLE_TYPE: %s", renderable);


      char surface[100] = "";

      if (info[i].surfaces & EGL_WINDOW_BIT)
         strcat(surface, "window,");
      if (info[i].surfaces & EGL_PBUFFER_BIT)
         strcat(surface, "pbuffer,");
      if (info[i].surfaces & EGL_PIXMAP_BIT)
         strcat(surface, "pixmap,");
      if (info[i].surfaces & EGL_STREAM_BIT_KHR)
         strcat(surface, "stream,");
      surface[strlen(surface) - 1] = '\0';

      printf("\n\tEGL_SURFACE_TYPE: %s", surface);
      printf("\n");
   }
}


/**
 * Print all available configurations.
 */
static void
PrintConfigs(EGLDisplay d, InfoMode info)
{
   EGLConfig configs[MAX_CONFIGS];
   struct eglconfig_info config_infos[MAX_CONFIGS];

   EGLint num_configs, i;

   eglGetConfigs(d, configs, MAX_CONFIGS, &num_configs);

   for (i = 0; i < num_configs; i++) {
      eglGetConfigAttrib(d, configs[i], EGL_CONFIG_ID, &config_infos[i].id);
      eglGetConfigAttrib(d, configs[i], EGL_BUFFER_SIZE, &config_infos[i].size);
      eglGetConfigAttrib(d, configs[i], EGL_LEVEL, &config_infos[i].level);

      eglGetConfigAttrib(d, configs[i], EGL_RED_SIZE, &config_infos[i].red);
      eglGetConfigAttrib(d, configs[i], EGL_GREEN_SIZE, &config_infos[i].green);
      eglGetConfigAttrib(d, configs[i], EGL_BLUE_SIZE, &config_infos[i].blue);
      eglGetConfigAttrib(d, configs[i], EGL_ALPHA_SIZE, &config_infos[i].alpha);
      eglGetConfigAttrib(d, configs[i], EGL_DEPTH_SIZE, &config_infos[i].depth);
      eglGetConfigAttrib(d, configs[i], EGL_STENCIL_SIZE, &config_infos[i].stencil);
      eglGetConfigAttrib(d, configs[i], EGL_NATIVE_VISUAL_ID, &config_infos[i].vid);
      eglGetConfigAttrib(d, configs[i], EGL_NATIVE_VISUAL_TYPE, &config_infos[i].vtype);

      eglGetConfigAttrib(d, configs[i], EGL_CONFIG_CAVEAT, &config_infos[i].caveat);
      eglGetConfigAttrib(d, configs[i], EGL_BIND_TO_TEXTURE_RGB, &config_infos[i].bind_rgb);
      eglGetConfigAttrib(d, configs[i], EGL_BIND_TO_TEXTURE_RGBA, &config_infos[i].bind_rgba);
      eglGetConfigAttrib(d, configs[i], EGL_RENDERABLE_TYPE, &config_infos[i].renderable);
      eglGetConfigAttrib(d, configs[i], EGL_SURFACE_TYPE, &config_infos[i].surfaces);

      eglGetConfigAttrib(d, configs[i], EGL_SAMPLES, &config_infos[i].samples);
      eglGetConfigAttrib(d, configs[i], EGL_SAMPLE_BUFFERS, &config_infos[i].sample_buffers);
   }

   if (info == Normal)
      PrintConfigsNormal(num_configs, config_infos);
   else if (info == Verbose)
      PrintConfigsVerbose(num_configs, config_infos);
}

static const char *
PrintDisplayExtensions(EGLDisplay d, EGLBoolean single_line)
{
   const char *extensions;

   extensions = eglQueryString(d, EGL_EXTENSIONS);
   if (!extensions)
      return NULL;

   if (strstr(extensions, "EGL_MESA_query_driver")) {
      printf("EGL driver name: %s\n", eglGetDisplayDriverName(d));
   }

   puts(d == EGL_NO_DISPLAY ? "EGL client extensions string:" :
                              "EGL extensions string:");

   print_extension_list(extensions, single_line);
   return extensions;
}


static const char *
PrintDeviceExtensions(EGLDeviceEXT d, EGLBoolean single_line)
{
   const char *extensions;

   puts("EGL device extensions string:");

   extensions = eglQueryDeviceStringEXT(d, EGL_EXTENSIONS);
   if (!extensions)
      return NULL;

   print_extension_list(extensions, single_line);
   return extensions;
}


static const char*
PrintContextExtensions(const char *api_name, EGLBoolean single_line)
{
   printf("%s extensions:\n", api_name);

   const char *extensions;

   if (GLAD_GL_VERSION_3_0) {
      struct ext_functions funcs = {
         .GetStringi = glGetStringi,
      };

      extensions = build_core_profile_extension_list(&funcs);
   } else {
      extensions = (const char*) glGetString(GL_EXTENSIONS);
   }

   print_extension_list(extensions, single_line);

   return extensions;
}


static EGLConfig
chooseEGLConfig(EGLDisplay d, int api_bitmask)
{
   const int attribs[] = {
      EGL_CONFORMANT,      api_bitmask,
      EGL_RED_SIZE,        1,
      EGL_GREEN_SIZE,      1,
      EGL_BLUE_SIZE,       1,
      EGL_ALPHA_SIZE,      1,
      EGL_RENDERABLE_TYPE, api_bitmask,
      EGL_NONE
   };

   int num_configs = 0;
   EGLConfig configs[MAX_CONFIGS];
   eglChooseConfig(d, attribs, configs, MAX_CONFIGS, &num_configs);

   if (num_configs == 0) {
      return NULL;
   } else {
      return configs[0];
   }
}

static EGLContext
createEGLContext(EGLDisplay d, EGLConfig conf, int api,
                 EGLBoolean khr_create_context,
                 EGLBoolean core_profile,
                 int *context_version)
{
   EGLContext ctx;

   /* can't create core GL context without KHR_create_context */
   if (core_profile && !khr_create_context) {
      return NULL;
   }

   if (api == EGL_OPENGL_API) {
      for (int i = 0; gl_versions[i].major > 0; i++) {
         /* if requesting for core profile, don't bother below GL 3.0 */
         if (core_profile &&
             gl_versions[i].major == 3 &&
             gl_versions[i].minor == 0)
            return NULL;

         const int attribs_new[] = {
            EGL_CONTEXT_MAJOR_VERSION, gl_versions[i].major,
            EGL_CONTEXT_MINOR_VERSION, gl_versions[i].minor,
            EGL_CONTEXT_OPENGL_PROFILE_MASK,
            core_profile ? EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT :
                           EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
            EGL_NONE,
         };

         const int attribs_old[] = {
            EGL_CONTEXT_CLIENT_VERSION, gl_versions[i].major,
            EGL_NONE,
         };

         const int *attribs = khr_create_context ? attribs_new : attribs_old;

         ctx = eglCreateContext(d, conf, EGL_NO_CONTEXT, attribs);

         if (ctx) {
            if (!eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
               eglDestroyContext(d, ctx);
               continue;
            }
            gladLoadGL((GLADloadfunc) eglGetProcAddress);
            *context_version =
               gl_versions[i].major * 10 + gl_versions[i].minor;
            return ctx;
         }
      }
      /* couldn't get core profile context */
      *context_version = 0;
      return NULL;
   }

   if (api == EGL_OPENGL_ES_API) {
      /* start from GLES3, then try to create context until we succeed
       * or reach GLES1
       */
      for (int i = 3; i > 0; i--) {
         const int attribs[] = {
            EGL_CONTEXT_MAJOR_VERSION, i,
            EGL_NONE,
         };
         ctx = eglCreateContext(d, conf, EGL_NO_CONTEXT, attribs);

         if (ctx) {
            if (!eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
               eglDestroyContext(d, ctx);
               continue;
            }

            if (i >= 2)
               gladLoadGLES2((GLADloadfunc) eglGetProcAddress);
            else
               gladLoadGLES1((GLADloadfunc) eglGetProcAddress);
            return ctx;
         }
      }
   }

   return NULL;
}

static int
doOneContext(EGLDisplay d,
             EGLContext ctx,
             const char *api_name,
             int version,
             struct options opts)
{
   if (!glGetString || !glGetIntegerv)
      return 1;

   printf("%s vendor: %s\n", api_name, glGetString(GL_VENDOR));
   printf("%s renderer: %s\n", api_name, glGetString(GL_RENDERER));
   printf("%s version: %s\n", api_name, glGetString(GL_VERSION));
   printf("%s shading language version: %s\n", api_name,
          glGetString(GL_SHADING_LANGUAGE_VERSION));

   const char *extensions = NULL;
   if (opts.mode != Brief) {
      extensions = PrintContextExtensions(api_name, opts.single_line);

      if (!extensions)
         return 1;

      print_gpu_memory_info(extensions);

      if (opts.limits) {
         struct ext_functions funcs = {
            .GetProgramivARB = glGetProgramivARB,
            .GetStringi = glGetStringi,
            .GetConvolutionParameteriv = glGetConvolutionParameteriv,
         };
         print_limits(extensions, api_name, version, &funcs);
      }
   }

   eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   gladLoaderUnloadGL();
   return 0;
}


static int
doOneDisplay(EGLDisplay d, const char *name, struct options opts)
{
   int maj, min;

   printf("%s platform:\n", name);
   if (!eglInitialize(d, &maj, &min)) {
      printf("eglinfo: eglInitialize failed\n\n");
      return 1;
   }

   if (!gladLoaderLoadEGL(d)) {
      printf("eglinfo: unable to load EGL functions for %s\n", name);
      return 1;
   }

   printf("EGL API version: %d.%d\n", maj, min);
   printf("EGL vendor string: %s\n", eglQueryString(d, EGL_VENDOR));
   printf("EGL version string: %s\n", eglQueryString(d, EGL_VERSION));

   const char *client_apis = eglQueryString(d, EGL_CLIENT_APIS);
   printf("EGL client APIs: %s\n", client_apis);

   const char *display_exts = eglQueryString(d, EGL_EXTENSIONS);

   if (opts.mode != Brief)
      PrintDisplayExtensions(d, opts.single_line);

   int khr_create_context = (maj == 1 && min >= 4) &&
      strstr(display_exts, "EGL_KHR_create_context") != 0;

   const char *has_opengl = strstr(client_apis, "OpenGL");
   const char *has_opengl_es = strstr(client_apis, "OpenGL_ES");
   const char *has_openvg = strstr(client_apis, "OpenVG");

   if (has_opengl == has_opengl_es) {
      int offset = strlen("OpenGL_ES");
      has_opengl = strstr(has_opengl_es + offset, "OpenGL");
   }

   EGLBoolean do_opengl_core =
      (opts.api == OPENGL || opts.api == OPENGL_CORE || opts.api == ALL);
   EGLBoolean do_opengl_compat = (opts.api == OPENGL || opts.api == ALL);
   EGLBoolean do_opengl_es = (opts.api == OPENGL_ES || opts.api == ALL);

   int version;

   if (has_opengl && (do_opengl_core || do_opengl_compat)) {
      EGLBoolean api_result = eglBindAPI(EGL_OPENGL_API);
      if (api_result) {
         EGLConfig config = chooseEGLConfig(d, EGL_OPENGL_BIT);
         EGLContext ctx = NULL;

         if (khr_create_context && do_opengl_core) {
            ctx = createEGLContext(d,
                                   config,
                                   EGL_OPENGL_API,
                                   EGL_TRUE,
                                   EGL_TRUE,
                                   &version);

            if (ctx)
               if (doOneContext(d, ctx, "OpenGL core profile", version, opts) == 0)
                  if (!eglDestroyContext(d, ctx))
                     return 1;
         }

         if (do_opengl_compat) {
            ctx = createEGLContext(d,
                                   config,
                                   EGL_OPENGL_API,
                                   khr_create_context,
                                   EGL_FALSE,
                                   &version);
            if (ctx)
               if (doOneContext(d, ctx, "OpenGL compatibility profile", version, opts) == 0)
                  if (!eglDestroyContext(d, ctx))
                     return 1;
         }
      }
   }

   if (has_opengl_es && do_opengl_es) {
      EGLBoolean api_result = eglBindAPI(EGL_OPENGL_ES_API);
      if (api_result) {
         EGLConfig config = chooseEGLConfig(d, EGL_OPENGL_ES_BIT);
         EGLContext ctx = createEGLContext(d,
                                           config,
                                           EGL_OPENGL_ES_API,
                                           khr_create_context,
                                           EGL_FALSE,
                                           &version);

         if (ctx) {
            if (doOneContext(d, ctx, "OpenGL ES profile", version, opts) == 0)
               if (!eglDestroyContext(d, ctx))
                  return 1;
         }
      }
   }
   if (has_openvg) {
      /* TODO: support OpenVG? */
   }

   if (opts.mode != Brief)
      PrintConfigs(d, opts.mode);

   eglTerminate(d);
   printf("\n");
   return 0;
}


static int
doOneDevice(EGLDeviceEXT d, int i, struct options opts)
{

   printf("Device #%d:\n\n", i);

   if (opts.mode != Brief)
      PrintDeviceExtensions(d, opts.single_line);

   return doOneDisplay(eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, d, NULL),
                       "Platform Device", opts);
}


static int
doDevices(const char *name, struct options opts)
{
   EGLDeviceEXT *devices;
   EGLint max_devices, num_devices;
   EGLint i;
   int ret = 0;

   printf("%s:\n", name);

   if (!eglQueryDevicesEXT(0, NULL, &max_devices))
      return 1;
   devices = calloc(sizeof(EGLDeviceEXT), max_devices);
   if (!devices)
      return 1;
   if (!eglQueryDevicesEXT(max_devices, devices, &num_devices))
     num_devices = 0;

   for (i = 0; i < num_devices; ++i) {
       ret += doOneDevice(devices[i], i, opts);
   }

   free(devices);

   return ret;
}


static void
usage(void)
{
   /*
    * Usage portion of the help message
    */

   printf("Usage: eglinfo [-h] [-B] [-s] [-v]");
   printf(" [-l]");
   printf(" [-a <api>]");
   printf(" [-p <platform>]\n");

   /*
    * Detailed portion of the help message
    */

   printf("\t -h \t This message.\n");
   printf("\t -B \t Brief output, print only the basics.\n");
   printf("\t -s \t Print a single extension per line.\n");
   printf("\t -v \t Print visuals info in verbose form.\n");
   printf("\t -l \t Print interesting OpenGL limits.\n");

   printf("\t -a \t Print information for a specific API, if supported.\n");
   printf("\t\t (");
   for (int i = 0; i < ELEMENTS(apis) - 1; i++) {
      printf("%s, ", apis[i]);
   }
   printf("%s)\n", apis[ELEMENTS(apis) - 1]);

   printf("\t -p \t Print information for a specific platform, if supported.\n");
   printf("\t\t (");
   for (int i = 0; i < ELEMENTS(platforms) - 1; i++) {
      printf("%s, ", platforms[i].short_name);
   }
   printf("%s)\n", platforms[ELEMENTS(platforms) - 1].short_name);
}

static void
parse_args(int argc, char *argv[], struct options *opts)
{
   opts->api = ALL;
   opts->platform = ALL; /* ALL == ~0 */
   opts->mode = Normal;
   opts->single_line = 0;
   opts->limits = 0;

   if (argc <= 1)
      return;

   for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-h") == 0) {
         usage();
         exit(0);
      }
   }

   for (int i = 1; i < argc; i++) {
      /* parse -l */
      if (strcmp(argv[i], "-l") == 0) {
         opts->limits = 1;
      }

      /* parse -a */
      else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
         const char *selected_api = argv[++i];

         for (int j = 0; j < ELEMENTS(apis); j++) {
            if (strcmp(selected_api, apis[j]) == 0) {
               opts->api = j;
               break;
            }
         }

         if (opts->api == ALL) {
            printf("Unknown API: %s\n", argv[i]);
            goto fail;
         }
      }

      /* parse -p */
      else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
         const char *selected_platform = argv[++i];

         for (int j = 0; j < ELEMENTS(platforms); j++) {
            if (strcmp(selected_platform, platforms[j].short_name) == 0) {
               opts->platform = j;
               break;
            }
         }

         if (opts->platform == ALL) {
            printf("Unknown platform: %s\n", argv[i]);
            goto fail;
         }
      }

      /* parse -B */
      else if (strcmp(argv[i], "-B") == 0) {
         opts->mode = Brief;
      }

      /* parse -s */
      else if (strcmp(argv[i], "-s") == 0) {
         opts->single_line = 1;
      }

      else if (strcmp(argv[i], "-v") == 0) {
         opts->mode = Verbose;
      }

      /* unknown */
      else {
         printf("Unknown option: %s\n", argv[i]);
         goto fail;
      }
   }

   return;

fail:
   usage();
   exit(1);
}

static int
doExtExplicitDevice(struct options opts, const char *clientext)
{
   int ret = 0;
   EGLDeviceEXT *devices;
   EGLint max_devices, num_devices;

   if (opts.platform != ALL)
      return 0;

   if (!eglQueryDevicesEXT(0, NULL, &max_devices)) {
      printf("eglinfo: queryDevices failed\n");
      return 1;
   }

   devices = calloc(sizeof(EGLDeviceEXT), max_devices);
   if (!devices) {
      printf("eglinfo: calloc failed\n");
      return 1;
   }

   if (!eglQueryDevicesEXT(max_devices, devices, &num_devices))
      num_devices = 0;

   for (int i = 0; i < ELEMENTS(platforms); i++) {
      for (int j = 0; j < ELEMENTS(platforms[i].names); j++) {
         const char *name = platforms[i].names[j];

         if (!name)
            break;

         if (!strstr(clientext, name))
            break;

         for (int k = 0; k < num_devices; k++) {
            const EGLint attrib_list[] = {
               EGL_DEVICE_EXT, (EGLAttrib) devices[k],
               EGL_NONE
            };
            char description[64];

            snprintf(description, 64, "Device %d on %s",
                     k, platforms[i].human_name);

            EGLDisplay d = eglGetPlatformDisplayEXT(platforms[i].platform_enum,
                                                    EGL_DEFAULT_DISPLAY,
                                                    attrib_list);
            if (!d)
               break;

            ret += doOneDisplay(d, description, opts);
         }
      }
   }

   free(devices);

   return ret;
}

static int
doExtPlatformBase(struct options opts, const char *clientext)
{
   bool found_platform_ext = false;
   int ret = 0;

   for (int i = 0; i < ELEMENTS(platforms); i++) {
      if (opts.platform != ALL && i != opts.platform)
         continue;

      for (int j = 0; j < ELEMENTS(platforms[i].names); j++) {
         const char *name = platforms[i].names[j];

         if (!name)
            break;

         if (strstr(clientext, name)) {
            EGLDisplay d = eglGetPlatformDisplayEXT(platforms[i].platform_enum,
                                                    EGL_DEFAULT_DISPLAY,
                                                    NULL);
            ret += doOneDisplay(d, platforms[i].human_name, opts);
            found_platform_ext = true;
            break;
         }
      }
   }

   if (!found_platform_ext)
      return -1;

   if (strstr(clientext, "EGL_EXT_device_enumeration") &&
       strstr(clientext, "EGL_EXT_platform_device") &&
       opts.platform == ALL)
      ret += doDevices("Device platform", opts);

   return ret;
}

int
main(int argc, char *argv[])
{
   struct options opts;
   parse_args(argc, argv, &opts);

   int ret = 0;
   const char *clientext;

   gladLoaderLoadEGL(EGL_NO_DISPLAY);

   if (opts.mode != Brief) {
      clientext = PrintDisplayExtensions(EGL_NO_DISPLAY, opts.single_line);
      printf("\n");
   } else {
      clientext = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
   }

   if (strstr(clientext, "EGL_EXT_explicit_device")) {
      ret += doExtExplicitDevice(opts, clientext);
   }

   int platform_base_ret;
   if (strstr(clientext, "EGL_EXT_platform_base") &&
       (platform_base_ret = doExtPlatformBase(opts, clientext)) >= 0) {
      ret += platform_base_ret;
   } else {
      ret = doOneDisplay(eglGetDisplay(EGL_DEFAULT_DISPLAY), "Default display", opts);
   }

   gladLoaderUnloadEGL();
   return ret;
}
