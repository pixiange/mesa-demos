/* readtex.c */

/*
 * Read a PNG image file and generate a mipmap texture set.
 */



#include "gl_wrap.h"
#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "readtex.h"

/*
** RGB Image Structure
*/

typedef struct _PNGImageRec {
   GLint sizeX, sizeY;
   GLint components;
   unsigned char *data;
} PNGImageRec;

static PNGImageRec *PNGImageLoad(const char *fileName)
{
   png_image image;
   memset(&image, 0, sizeof(image));
   image.version = PNG_IMAGE_VERSION;
   PNGImageRec *final = NULL;
   unsigned char *data = NULL;

   if (png_image_begin_read_from_file(&image, fileName) == 0) {
      fprintf(stderr, "Failed to read PNG image header for file '%s'\n", fileName);
      goto error;
   }

   int has_alpha = (image.flags & PNG_FORMAT_FLAG_ALPHA) != 0;
   if (image.width > 16384 || image.height > 16384) {
      /* Very large images can break old code using `int` instead of `size_t` */
      fprintf(stderr, "Image size too big\n");
      goto error;
   }

   image.format = has_alpha ? PNG_FORMAT_RGBA : PNG_FORMAT_RGB;

   int components = has_alpha ? 4 : 3;
   int stride = image.width * components;
   data = calloc(image.height, stride);
   if (!data) {
      fprintf(stderr, "Out of memory!\n");
      goto error;
   }

   // Use negative stride to place bottom row first
   if (png_image_finish_read(&image, NULL, data, -stride, NULL) == 0) {
      fprintf(stderr, "Failed to read PNG image contents for file '%s'\n", fileName);
      goto error;
   }

   final = (PNGImageRec *)calloc(1, sizeof(PNGImageRec));
   if (final == NULL) {
      fprintf(stderr, "Out of memory!\n");
      goto error;
   }
   final->sizeX = image.width;
   final->sizeY = image.height;
   final->components = components;
   final->data = data;
   return final;

error:
   png_image_free(&image);
   free(data);
   free(final);
   return NULL;
}


static void FreeImage( PNGImageRec *image )
{
   free(image->data);
   free(image);
}


/*
 * Load an PNG .png file and generate a set of 2-D mipmaps from it.
 * Input:  imageFile - name of .png to read
 *         intFormat - internal texture format to use, or number of components
 * Return:  GL_TRUE if success, GL_FALSE if error.
 */
GLboolean LoadRGBMipmaps( const char *imageFile, GLint intFormat )
{
   GLint w, h;
   return LoadRGBMipmaps2( imageFile, GL_TEXTURE_2D, intFormat, &w, &h );
}



GLboolean LoadRGBMipmaps2( const char *imageFile, GLenum target,
                           GLint intFormat, GLint *width, GLint *height )
{
   GLint error;
   GLenum format;
   PNGImageRec *image;

   image = PNGImageLoad( imageFile );
   if (!image) {
      return GL_FALSE;
   }

   if (image->components==3) {
      format = GL_RGB;
   }
   else if (image->components==4) {
      format = GL_RGBA;
   }
   else {
      /* not implemented */
      fprintf(stderr,
              "Error in LoadRGBMipmaps %d-component images not implemented\n",
              image->components );
      FreeImage(image);
      return GL_FALSE;
   }

   error = gluBuild2DMipmaps( target,
                              intFormat,
                              image->sizeX, image->sizeY,
                              format,
                              GL_UNSIGNED_BYTE,
                              image->data );

   *width = image->sizeX;
   *height = image->sizeY;

   FreeImage(image);

   return error ? GL_FALSE : GL_TRUE;
}



/*
 * Load an PNG .png file and return a pointer to the image data.
 * Input:  imageFile - name of .png to read
 * Output:  width - width of image
 *          height - height of image
 *          format - format of image (GL_RGB or GL_RGBA)
 * Return:  pointer to image data or NULL if error
 */
GLubyte *LoadRGBImage( const char *imageFile, GLint *width, GLint *height,
                       GLenum *format )
{
   PNGImageRec *image;
   GLint bytes;
   GLubyte *buffer;

   image = PNGImageLoad( imageFile );
   if (!image) {
      return NULL;
   }

   if (image->components==3) {
      *format = GL_RGB;
   }
   else if (image->components==4) {
      *format = GL_RGBA;
   }
   else {
      /* not implemented */
      fprintf(stderr,
              "Error in LoadRGBImage %d-component images not implemented\n",
              image->components );
      FreeImage(image);
      return NULL;
   }

   *width = image->sizeX;
   *height = image->sizeY;

   bytes = image->sizeX * image->sizeY * image->components;
   buffer = (GLubyte *) malloc(bytes);
   if (!buffer) {
      FreeImage(image);
      return NULL;
   }

   memcpy( (void *) buffer, (void *) image->data, bytes );

   FreeImage(image);

   return buffer;
}

#define CLAMP( X, MIN, MAX )  ( (X)<(MIN) ? (MIN) : ((X)>(MAX) ? (MAX) : (X)) )


static void ConvertRGBtoYUV(GLint w, GLint h, GLint texel_bytes,
			    const GLubyte *src,
                            GLushort *dest)
{
   GLint i, j;

   for (i = 0; i < h; i++) {
      for (j = 0; j < w; j++) {
         const GLfloat r = src[0] / 255.0f;
         const GLfloat g = src[1] / 255.0f;
         const GLfloat b = src[2] / 255.0f;
         GLfloat y, cr, cb;
         GLint iy, icr, icb;

         y  = r * 65.481f + g * 128.553f + b * 24.966f + 16;
         cb = r * -37.797f + g * -74.203f + b * 112.0f + 128;
         cr = r * 112.0f + g * -93.786f + b * -18.214f + 128;
         /*printf("%f %f %f -> %f %f %f\n", r, g, b, y, cb, cr);*/
         iy  = (GLint) CLAMP(y,  0, 254);
         icb = (GLint) CLAMP(cb, 0, 254);
         icr = (GLint) CLAMP(cr, 0, 254);

         if (j & 1) {
            /* odd */
            *dest = (GLushort)((iy << 8) | icr);
         }
         else {
            /* even */
            *dest = (GLushort)((iy << 8) | icb);
         }
         dest++;
	 src += texel_bytes;
      }
   }
}


/*
 * Load an PNG .png file and return a pointer to the image data, converted
 * to 422 yuv.
 *
 * Input:  imageFile - name of .png to read
 * Output:  width - width of image
 *          height - height of image
 * Return:  pointer to image data or NULL if error
 */
GLushort *LoadYUVImage( const char *imageFile, GLint *width, GLint *height )
{
   PNGImageRec *image;
   GLushort *buffer;

   image = PNGImageLoad( imageFile );
   if (!image) {
      return NULL;
   }

   if (image->components != 3 && image->components !=4 ) {
      /* not implemented */
      fprintf(stderr,
              "Error in LoadYUVImage %d-component images not implemented\n",
              image->components );
      FreeImage(image);
      return NULL;
   }

   *width = image->sizeX;
   *height = image->sizeY;

   buffer = (GLushort *) malloc( image->sizeX * image->sizeY * 2 );

   if (buffer)
      ConvertRGBtoYUV( image->sizeX,
		       image->sizeY,
		       image->components,
		       image->data,
		       buffer );


   FreeImage(image);
   return buffer;
}

