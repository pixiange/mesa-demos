/*
 * Copyright (C) 1999-2001  Brian Paul
 * Copyright (C) 2010  Kristian Høgsberg
 * Copyright (C) 2010  Chia-I Wu
 * Copyright (C) 2023  Collabora Ltd
 *
 * SPDX-License-Identifier: MIT
 */

#include "matrix.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

void
mat4_multiply(float m[16], const float n[16])
{
   float tmp[16];
   const float *row, *column;
   int i, j, k;

   for (j = 0; j < 4; j++) {
      for (i = 0; i < 4; i++) {
         float sum = 0.0f;
         row = n + j * 4;
         column = m + i;
         for (k = 0; k < 4; k++)
            sum += row[k] * column[k * 4];
         tmp[j * 4 + i] = sum;
      }
   }
   memcpy(m, &tmp, sizeof tmp);
}

void
mat4_scale(float m[16], float x, float y, float z)
{
   float s[16] = {
      x, 0, 0, 0,
      0, y, 0, 0,
      0, 0, z, 0,
      0, 0, 0, 1
   };
   mat4_multiply(m, s);
}

void
mat4_rotate(float m[16], float angle, float x, float y, float z)
{
   double s, c;
#if HAVE_SINCOS
   sincos(angle, &s, &c);
#else
   s = sin(angle);
   c = cos(angle);
#endif
   float r[16] = {
      x * x * (1 - c) + c,     y * x * (1 - c) + z * s, x * z * (1 - c) - y * s, 0,
      x * y * (1 - c) - z * s, y * y * (1 - c) + c,     y * z * (1 - c) + x * s, 0,
      x * z * (1 - c) + y * s, y * z * (1 - c) - x * s, z * z * (1 - c) + c,     0,
      0, 0, 0, 1
   };

   mat4_multiply(m, r);
}

void
mat4_translate(float m[16], float x, float y, float z)
{
   float t[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  x, y, z, 1 };

   mat4_multiply(m, t);
}

void
mat4_identity(float m[16])
{
   float t[16] = {
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0,
   };

   memcpy(m, t, sizeof(t));
}

void
mat4_transpose(float m[16])
{
   float t[16] = {
      m[0], m[4], m[8],  m[12],
      m[1], m[5], m[9],  m[13],
      m[2], m[6], m[10], m[14],
      m[3], m[7], m[11], m[15]};

   memcpy(m, t, sizeof(t));
}

void
mat4_invert(float m[16])
{
   float t[16];
   mat4_identity(t);

   // Extract and invert the translation part 't'. The inverse of a
   // translation matrix can be calculated by negating the translation
   // coordinates.
   t[12] = -m[12]; t[13] = -m[13]; t[14] = -m[14];

   // Invert the rotation part 'r'. The inverse of a rotation matrix is
   // equal to its transpose.
   m[12] = m[13] = m[14] = 0;
   mat4_transpose(m);

   // inv(m) = inv(r) * inv(t)
   mat4_multiply(m, t);
}

void
mat4_frustum_gl(float m[16], float l, float r, float b, float t, float n, float f)
{
   float tmp[16];
   mat4_identity(tmp);

   float deltaX = r - l;
   float deltaY = t - b;
   float deltaZ = f - n;

   tmp[0] = (2 * n) / deltaX;
   tmp[5] = (2 * n) / deltaY;
   tmp[8] = (r + l) / deltaX;
   tmp[9] = (t + b) / deltaY;
   tmp[10] = -(f + n) / deltaZ;
   tmp[11] = -1;
   tmp[14] = -(2 * f * n) / deltaZ;
   tmp[15] = 0;

   memcpy(m, tmp, sizeof(tmp));
}

void
mat4_frustum_vk(float m[16], float l, float r, float b, float t, float n, float f)
{
   float tmp[16];
   mat4_identity(tmp);

   float deltaX = r - l;
   float deltaY = t - b;
   float deltaZ = f - n;

   tmp[0] = (2 * n) / deltaX;
   tmp[5] = (-2 * n) / deltaY;
   tmp[8] = (r + l) / deltaX;
   tmp[9] = (t + b) / deltaY;
   tmp[10] = f / (n - f);
   tmp[11] = -1;
   tmp[14] = -(f * n) / deltaZ;
   tmp[15] = 0;

   memcpy(m, tmp, sizeof(tmp));
}

void
mat4_perspective_gl(float m[16], float fovy, float aspect,
                    float zNear, float zFar)
{
   float tmp[16];
   mat4_identity(tmp);

   double sine, cosine, cotangent, deltaZ;
   float radians = fovy / 2 * M_PI / 180;

   deltaZ = zFar - zNear;
   sine = sin(radians);
   cosine = cos(radians);

   if ((deltaZ == 0) || (sine == 0) || (aspect == 0))
      return;

   cotangent = cosine / sine;

   tmp[0] = cotangent / aspect;
   tmp[5] = cotangent;
   tmp[10] = -(zFar + zNear) / deltaZ;
   tmp[11] = -1;
   tmp[14] = -2 * zNear * zFar / deltaZ;
   tmp[15] = 0;

   memcpy(m, tmp, sizeof(tmp));
}
