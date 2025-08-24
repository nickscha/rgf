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
#define RGF_API static
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
/* # Utility String functions                                 */
/* ########################################################## */
RGF_API RGF_INLINE int rgf_is_space(char c)
{
  return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

RGF_API RGF_INLINE int rgf_atoi(char *s, int *consumed)
{
  int sign = 1;
  int value = 0;
  int i = 0;

  if (s[i] == '-')
  {
    sign = -1;
    i++;
  }
  else if (s[i] == '+')
  {
    i++;
  }

  while (s[i] >= '0' && s[i] <= '9')
  {
    value = value * 10 + (s[i] - '0');
    i++;
  }

  *consumed = i;

  return value * sign;
}

RGF_API RGF_INLINE float rgf_atof(char *s, int *consumed)
{
  int i = 0;
  int sign = 1;
  float value = 0.0f;
  float frac = 0.0f;
  float divisor = 1.0f;

  if (s[i] == '-')
  {
    sign = -1;
    i++;
  }
  else if (s[i] == '+')
  {
    i++;
  }

  while (s[i] >= '0' && s[i] <= '9')
  {
    value = value * 10.0f + (float)(s[i] - '0');
    i++;
  }

  if (s[i] == '.')
  {
    i++;
    while (s[i] >= '0' && s[i] <= '9')
    {
      frac = frac * 10.0f + (float)(s[i] - '0');
      divisor *= 10.0f;
      i++;
    }
    value += frac / divisor;
  }

  *consumed = i;

  return value * (float)sign;
}

/* ########################################################## */
/* # OBJ to RGF conversion funciton                           */
/* ########################################################## */
#ifndef RGF_OBJ_MAX_FACE_VERTICES
#define RGF_OBJ_MAX_FACE_VERTICES 8
#endif

RGF_API RGF_INLINE int rgf_parse_obj(
    rgf_model *model,             /* The filled by supplied obj file data model      */
    unsigned char *obj_binary,    /* OBJ File binary buffer                          */
    unsigned long obj_binary_size /* OBJ File binary buffer size                     */
)
{
  unsigned long i = 0;
  unsigned long vertex_count = 0;
  unsigned long triangle_count = 0;
  unsigned long v_index = 0;
  unsigned long f_index = 0;
  int face_indices_temp[RGF_OBJ_MAX_FACE_VERTICES];
  int face_vertices_count;

  /* Check input arguments */
  if (!model || !obj_binary || obj_binary_size <= 0)
  {
    return 0;
  }

  /* First pass: count vertices & triangles */
  i = 0;

  while (i < obj_binary_size)
  {
    if (obj_binary[i] == 'v' && obj_binary[i + 1] == ' ')
    {
      vertex_count++;
    }
    else if (obj_binary[i] == 'f' && obj_binary[i + 1] == ' ')
    {
      unsigned long j = i + 2;
      int current_char_is_digit;

      face_vertices_count = 0;

      while (j < obj_binary_size && obj_binary[j] != '\n' && obj_binary[j] != '\r')
      {
        current_char_is_digit = obj_binary[j] >= '0' && obj_binary[j] <= '9';

        if (current_char_is_digit || obj_binary[j] == '-')
        {
          face_vertices_count++;
          /* Skip to end of number, ignoring slashes */
          while (j < obj_binary_size && obj_binary[j] != ' ' && obj_binary[j] != '\n' && obj_binary[j] != '\r')
          {
            j++;
          }
        }
        else
        {
          j++;
        }
      }

      if (face_vertices_count >= 3)
      {
        triangle_count += (unsigned long)(face_vertices_count - 2);
      }
    }

    while (i < obj_binary_size && obj_binary[i] != '\n')
    {
      i++;
    }
    i++;
  }

  /* Allocate arrays */
  model->vertices_size = vertex_count * 3;
  model->indices_size = triangle_count * 3;

  if (!model->vertices || !model->indices)
  {
    return 0;
  }

  /* Second pass: parse data */
  i = 0;
  model->min_x = model->min_y = model->min_z = 1e30f;
  model->max_x = model->max_y = model->max_z = -1e30f;

  while (i < obj_binary_size)
  {
    if (obj_binary[i] == 'v' && obj_binary[i + 1] == ' ')
    {
      int consumed = 0;

      i += 2;

      model->vertices[v_index] = rgf_atof((char *)(obj_binary + i), &consumed);

      if (model->vertices[v_index] < model->min_x)
      {
        model->min_x = model->vertices[v_index];
      }

      if (model->vertices[v_index] > model->max_x)
      {
        model->max_x = model->vertices[v_index];
      }

      i += (unsigned long)consumed;
      v_index++;

      while (rgf_is_space((char)obj_binary[i]))
      {
        i++;
      }

      model->vertices[v_index] = rgf_atof((char *)(obj_binary + i), &consumed);

      if (model->vertices[v_index] < model->min_y)
      {
        model->min_y = model->vertices[v_index];
      }

      if (model->vertices[v_index] > model->max_y)
      {
        model->max_y = model->vertices[v_index];
      }

      i += (unsigned long)consumed;
      v_index++;

      while (rgf_is_space((char)obj_binary[i]))
      {
        i++;
      }

      model->vertices[v_index] = rgf_atof((char *)(obj_binary + i), &consumed);

      if (model->vertices[v_index] < model->min_z)
      {
        model->min_z = model->vertices[v_index];
      }

      if (model->vertices[v_index] > model->max_z)
      {
        model->max_z = model->vertices[v_index];
      }

      i += (unsigned long)consumed;
      v_index++;
    }
    else if (obj_binary[i] == 'f' && obj_binary[i + 1] == ' ')
    {
      int consumed = 0;
      int j;
      int current_index;

      i += 2;
      face_vertices_count = 0;

      /* Parse all vertex indices on the line */
      while (i < obj_binary_size && obj_binary[i] != '\n' && obj_binary[i] != '\r' && face_vertices_count < RGF_OBJ_MAX_FACE_VERTICES)
      {
        /* Skip non-numeric characters before the vertex index */
        while (i < obj_binary_size && rgf_is_space((char)obj_binary[i]))
        {
          i++;
        }

        if (i >= obj_binary_size || obj_binary[i] == '\n' || obj_binary[i] == '\r')
        {
          break;
        }

        current_index = rgf_atoi((char *)(obj_binary + i), &consumed);
        i += (unsigned long)consumed;

        /* Skip any texture/normal indices and the trailing space/newline */
        while (i < obj_binary_size && !rgf_is_space((char)obj_binary[i]) && obj_binary[i] != '\n' && obj_binary[i] != '\r')
        {
          i++;
        }

        /* Handle negative indices (relative to the end of the vertex count) */
        if (current_index < 0)
        {
          current_index = (int)vertex_count + current_index;
        }
        else
        {
          current_index = current_index - 1; /* Convert to 0-based */
        }

        face_indices_temp[face_vertices_count] = current_index;
        face_vertices_count++;
      }

      /* Triangulate the face using a fan method */
      if (face_vertices_count >= 3)
      {
        for (j = 1; j < face_vertices_count - 1; j++)
        {
          model->indices[f_index++] = face_indices_temp[0];
          model->indices[f_index++] = face_indices_temp[j];
          model->indices[f_index++] = face_indices_temp[j + 1];
        }
      }
    }

    while (i < obj_binary_size && obj_binary[i] != '\n')
    {
      i++;
    }
    i++;
  }

  model->center_x = (model->min_x + model->max_x) * 0.5f;
  model->center_y = (model->min_y + model->max_y) * 0.5f;
  model->center_z = (model->min_z + model->max_z) * 0.5f;

  return 1;
}

/* ########################################################## */
/* # Geometry manipulation functions                          */
/* ########################################################## */
RGF_API RGF_INLINE void rgf_model_center(
    rgf_model *model,
    float center_x,
    float center_y,
    float center_z)
{
  float current_center_x, current_center_y, current_center_z;
  float offset_x, offset_y, offset_z;
  unsigned long i;

  /* Calculate the current center of the model's bounding box */
  current_center_x = (model->min_x + model->max_x) / 2.0f;
  current_center_y = (model->min_y + model->max_y) / 2.0f;
  current_center_z = (model->min_z + model->max_z) / 2.0f;

  /* Determine the offset needed to move to the new center */
  offset_x = center_x - current_center_x;
  offset_y = center_y - current_center_y;
  offset_z = center_z - current_center_z;

  /* Apply the offset to every vertex */
  for (i = 0; i < model->vertices_size; i += 3)
  {
    model->vertices[i + 0] += offset_x;
    model->vertices[i + 1] += offset_y;
    model->vertices[i + 2] += offset_z;
  }

  /* Update the model's bounding box and center properties */
  model->min_x += offset_x;
  model->max_x += offset_x;
  model->min_y += offset_y;
  model->max_y += offset_y;
  model->min_z += offset_z;
  model->max_z += offset_z;
  model->center_x = center_x;
  model->center_y = center_y;
  model->center_z = center_z;
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
  char *src8 = (char *)src;
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
