/* rgf.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) raw geometry format (RGF).

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef RGF_H
#define RGF_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define RGF_INLINE inline
#define RGF_API extern
#elif defined(__GNUC__) || defined(__clang__)
#define RGF_INLINE __inline__
#define RGF_API static
#elif defined(_MSC_VER)
#define RGF_INLINE __inline
#define RGF_API static
#else
#define RGF_INLINE
#define RGF_API static
#endif

/* ########################################################## */
/* # The standard RGF data model                              */
/* ########################################################## */
typedef struct rgf_model
{

  unsigned long vertices_size;
  unsigned long indices_size;

  float min_x;
  float min_y;
  float min_z;

  float max_x;
  float max_y;
  float max_z;

  float center_x;
  float center_y;
  float center_z;

  float *vertices;
  int *indices;

} rgf_model;

/* ########################################################## */
/* # OBJ to RGF conversion funciton                           */
/* ########################################################## */
RGF_API RGF_INLINE int rgf_convert_obj(
    rgf_model *model,              /* The filled by supplied obj file data model      */
    unsigned char *obj_binary,     /* OBJ File binary buffer                          */
    unsigned long obj_binary_size, /* OBJ File binary buffer size                     */
    float center_position[3],      /* Where should the model be cenetered             */
    float scale_target             /* Model normalization 1.0f = 1x1x1 dimension, ... */
)
{

  /* Check input arguments */
  if (!model || !obj_binary || obj_binary_size <= 0 || !center_position || scale_target <= 0.0f)
  {
    return 0;
  }

  return 1;
}

/* ########################################################## */
/* # Binary En-/Decoding of rgf data                          */
/* ########################################################## */
#define RGF_BINARY_VERSION 1
#define RGF_BINARY_SIZE_MAGIC 4
#define RGF_BINARY_SIZE_VERSION 4
#define RGF_BINARY_SIZE_HEADER (RGF_BINARY_SIZE_MAGIC + RGF_BINARY_SIZE_VERSION)

RGF_API RGF_INLINE void *rgf_binary_memcpy(void *dest, void *src, unsigned long count)
{
  char *dest8 = (char *)dest;
  const char *src8 = (char *)src;
  while (count--)
  {
    *dest8++ = *src8++;
  }

  return dest;
}

RGF_API RGF_INLINE int rgf_binary_encode(
    unsigned char *out_binary,         /* Output buffer for executable        */
    unsigned long out_binary_capacity, /* Capacity of output buffer           */
    unsigned long *out_binary_size,    /* Actual size of output binary buffer */
    rgf_model *model                   /* The rgf data                        */
)
{
  unsigned char *ptr = out_binary;

  unsigned long size_vertices = model->vertices_size * sizeof(float);
  unsigned long size_indices = model->indices_size * sizeof(int);
  unsigned long size_total = (unsigned long)(RGF_BINARY_SIZE_HEADER +    /* Header                                  */
                                             2 * sizeof(unsigned long) + /* Vertices/Indices count                  */
                                             9 * sizeof(int) +           /* Boundaries of geometry (min,max,center) */
                                             size_vertices +             /* Vertices data                           */
                                             size_indices                /* Indices data                            */
  );

  if (out_binary_capacity < size_total)
  {
    /* Binary buffer size cannot fit the rgf data */
    return 0;
  }

  /* 4 byte magic */
  ptr[0] = 'R';
  ptr[1] = 'G';
  ptr[2] = 'F';
  ptr[3] = '\0';

  /* 1 byte version + 3 byte padding */
  ptr[4] = RGF_BINARY_VERSION;
  ptr[5] = 0;
  ptr[6] = 0;
  ptr[7] = 0;

  ptr += RGF_BINARY_SIZE_HEADER;

  rgf_binary_memcpy(ptr, &model->vertices_size, (unsigned long)sizeof(unsigned long));
  ptr += sizeof(unsigned long);
  rgf_binary_memcpy(ptr, &model->indices_size, (unsigned long)sizeof(unsigned long));
  ptr += sizeof(unsigned long);

  rgf_binary_memcpy(ptr, &model->min_x, 4);
  ptr += 4;
  rgf_binary_memcpy(ptr, &model->min_y, 4);
  ptr += 4;
  rgf_binary_memcpy(ptr, &model->min_z, 4);
  ptr += 4;

  rgf_binary_memcpy(ptr, &model->max_x, 4);
  ptr += 4;
  rgf_binary_memcpy(ptr, &model->max_y, 4);
  ptr += 4;
  rgf_binary_memcpy(ptr, &model->max_z, 4);
  ptr += 4;

  rgf_binary_memcpy(ptr, &model->center_x, 4);
  ptr += 4;
  rgf_binary_memcpy(ptr, &model->center_y, 4);
  ptr += 4;
  rgf_binary_memcpy(ptr, &model->center_z, 4);
  ptr += 4;

  rgf_binary_memcpy(ptr, model->vertices, size_vertices);
  ptr += size_vertices;
  rgf_binary_memcpy(ptr, model->indices, size_indices);
  ptr += size_indices;

  *out_binary_size = size_total;

  return 1;
}

RGF_API RGF_INLINE unsigned long rgf_binary_read_ul(unsigned char *ptr)
{
  return ((unsigned long)ptr[0]) |
         ((unsigned long)ptr[1] << 8) |
         ((unsigned long)ptr[2] << 16) |
         ((unsigned long)ptr[3] << 24);
}

RGF_API RGF_INLINE float rgf_binary_read_float(unsigned char *ptr)
{
  union
  {
    unsigned long l;
    float f;
  } u;

  u.l = rgf_binary_read_ul(ptr);

  return u.f;
}

RGF_API RGF_INLINE int rgf_binary_decode(
    unsigned char *in_binary,     /* Output buffer for executable        */
    unsigned long in_binary_size, /* Actual size of output binary buffer */
    rgf_model *model              /* The rgf data model                  */
)
{
  unsigned char *binary_ptr;

  unsigned long size_total;

  if (in_binary_size < RGF_BINARY_SIZE_HEADER)
  {
    /* no valid rgf binary */
    return 0;
  }

  if (in_binary[0] != 'R' || in_binary[1] != 'G' || in_binary[2] != 'F' || in_binary[3] != '\0')
  {
    /* no right magic */
    return 0;
  }
  if (in_binary[4] != RGF_BINARY_VERSION)
  {
    /* no right version */
    return 0;
  }

  if (in_binary[5] != 0 || in_binary[6] != 0 || in_binary[7] != 0)
  {
    /* no right padding */
    return 0;
  }

  binary_ptr = in_binary + RGF_BINARY_SIZE_HEADER;

  model->vertices_size = rgf_binary_read_ul(binary_ptr);
  binary_ptr += sizeof(unsigned long);
  model->indices_size = rgf_binary_read_ul(binary_ptr);
  binary_ptr += sizeof(unsigned long);

  size_total = (unsigned long)(RGF_BINARY_SIZE_HEADER +               /* Header                                  */
                               2 * sizeof(unsigned long) +            /* Vertices/Indices count                  */
                               9 * sizeof(float) +                    /* Boundaries of geometry (min,max,center) */
                               model->vertices_size * sizeof(float) + /* Vertices data                           */
                               model->indices_size * sizeof(int)      /* Indices data                            */
  );

  if (in_binary_size < size_total)
  {
    /* no space for data */
    return 0;
  }

  model->min_x = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;
  model->min_y = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;
  model->min_z = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;

  model->max_x = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;
  model->max_y = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;
  model->max_z = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;

  model->center_x = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;
  model->center_y = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;
  model->center_z = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;

  model->vertices = (float *)binary_ptr;
  binary_ptr += model->vertices_size * sizeof(float);

  model->indices = (int *)binary_ptr;
  binary_ptr += model->indices_size * sizeof(int);

  return 1;
}

#endif /* RGF_H */

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2025 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/
