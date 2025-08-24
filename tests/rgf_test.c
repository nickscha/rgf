/* rgf.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) raw geometry format (RGF).

This Test class defines cases to verify that we don't break the excepted behaviours in the future upon changes.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include "../rgf.h"             /* Raw Geometry Format                                   */
#include "../rgf_platform_io.h" /* Optional: OS-Specific read/write file implementations */

#include "test.h" /* Simple Testing framework */

#define RGF_TEST_EPSILON 1e-6f
#define BINARY_BUFFER_CAPACITY 1024

void rgf_test_encode_decode(void)
{
  /* The encoded model data */
  unsigned char binary_buffer[BINARY_BUFFER_CAPACITY];
  unsigned long binary_buffer_size = 0;

  /* Setup of the rgf model data */
  float vertices[] = {1.0f, 0.0f};
  int indices[] = {0, 1};

  rgf_model binary_model = {0};

  rgf_model model = {0};
  model.vertices_size = 2;
  model.indices_size = 2;
  model.vertices = vertices;
  model.indices = indices;

  /* ########################################################## */
  /* # Endcode to binary                                        */
  /* ########################################################## */
  assert(rgf_binary_encode(binary_buffer, BINARY_BUFFER_CAPACITY, &binary_buffer_size, &model));

  /* Check header */
  assert(binary_buffer[0] == 'R');
  assert(binary_buffer[1] == 'G');
  assert(binary_buffer[2] == 'F');
  assert(binary_buffer[3] == '\0');
  assert(binary_buffer[4] == RGF_BINARY_VERSION);
  assert(binary_buffer[5] == 0);
  assert(binary_buffer[6] == 0);
  assert(binary_buffer[7] == 0);

  /* ########################################################## */
  /* # Decode from binary                                       */
  /* ########################################################## */
  assert(rgf_binary_decode(binary_buffer, binary_buffer_size, &binary_model));

  assert(binary_model.vertices_size == model.vertices_size);
  assert(binary_model.indices_size == model.indices_size);
}

void rgf_test_encode_to_file(void)
{
  /* The encoded model data */
  unsigned char binary_buffer[BINARY_BUFFER_CAPACITY];
  unsigned long binary_buffer_size = 0;

  /* Setup of the rgf model data */
  float vertices[] = {1.0f, 0.0f};
  int indices[] = {0, 1};

  rgf_model model = {0};
  model.vertices_size = 2;
  model.indices_size = 2;
  model.vertices = vertices;
  model.indices = indices;

  /* ########################################################## */
  /* # Endcode to binary                                        */
  /* ########################################################## */
  assert(rgf_binary_encode(binary_buffer, BINARY_BUFFER_CAPACITY, &binary_buffer_size, &model));

  /* Check header */
  assert(binary_buffer[0] == 'R');
  assert(binary_buffer[1] == 'G');
  assert(binary_buffer[2] == 'F');
  assert(binary_buffer[3] == '\0');
  assert(binary_buffer[4] == RGF_BINARY_VERSION);
  assert(binary_buffer[5] == 0);
  assert(binary_buffer[6] == 0);
  assert(binary_buffer[7] == 0);

  assert(rgf_platform_write("test.rgf", binary_buffer, binary_buffer_size));
}

void rgf_test_decode_from_file(void)
{
  /* The decoded model data */
  unsigned char binary_buffer[BINARY_BUFFER_CAPACITY];
  unsigned long binary_buffer_size = 0;

  rgf_model model = {0};

  assert(rgf_platform_read("test_v1.rgf", binary_buffer, BINARY_BUFFER_CAPACITY, &binary_buffer_size));

  /* Check header */
  assert(binary_buffer[0] == 'R');
  assert(binary_buffer[1] == 'G');
  assert(binary_buffer[2] == 'F');
  assert(binary_buffer[3] == '\0');
  assert(binary_buffer[4] == RGF_BINARY_VERSION);
  assert(binary_buffer[5] == 0);
  assert(binary_buffer[6] == 0);
  assert(binary_buffer[7] == 0);

  assert(rgf_binary_decode(binary_buffer, binary_buffer_size, &model));

  assert(model.vertices_size == 2);
  assert(model.indices_size == 2);

  assert_equalsf(model.vertices[0], 1.0f, RGF_TEST_EPSILON);
  assert_equalsf(model.vertices[1], 0.0f, RGF_TEST_EPSILON);
  assert(model.indices[0] == 0);
  assert(model.indices[1] == 1);
}

void rgf_test_parse_obj(void)
{
  /* Stack Memory */
  float vertices_buffer[30000];
  int indices_buffer[60000];
  unsigned char binary_buffer[1500000];
  unsigned long binary_buffer_size = 0;

  rgf_model model = {0};
  model.vertices = vertices_buffer;
  model.indices = indices_buffer;

  assert(rgf_platform_read("head.obj", binary_buffer, 1500000, &binary_buffer_size));
  assert(rgf_parse_obj(&model, binary_buffer, binary_buffer_size));

  assert(model.vertices_size > 0);
  assert(model.indices_size > 0);

  assert(model.vertices_size == 26532UL);
  assert(model.indices_size == 53052UL);

  /* Check vertices values */
  assert_equalsf(model.vertices[0], 0.028666f, RGF_TEST_EPSILON);
  assert_equalsf(model.vertices[1], 0.031898f, RGF_TEST_EPSILON);
  assert_equalsf(model.vertices[2], -0.184875f, RGF_TEST_EPSILON);

  assert_equalsf(model.vertices[model.vertices_size - 3], -0.077342f, RGF_TEST_EPSILON);
  assert_equalsf(model.vertices[model.vertices_size - 2], -0.000485f, RGF_TEST_EPSILON);
  assert_equalsf(model.vertices[model.vertices_size - 1], -0.071214f, RGF_TEST_EPSILON);

  /* Check indices triangulation (n-gons) */
  assert(model.indices[0] == 200);
  assert(model.indices[1] == 2189);
  assert(model.indices[2] == 2193);
  assert(model.indices[3] == 200);
  assert(model.indices[4] == 2193);
  assert(model.indices[5] == 2192);

  assert(model.indices[model.indices_size - 6] == 7904);
  assert(model.indices[model.indices_size - 5] == 8843);
  assert(model.indices[model.indices_size - 4] == 7906);
  assert(model.indices[model.indices_size - 3] == 7904);
  assert(model.indices[model.indices_size - 2] == 7906);
  assert(model.indices[model.indices_size - 1] == 1263);

  /* Check model boundaries */
  assert_equalsf(model.min_x, -0.221155f, RGF_TEST_EPSILON);
  assert_equalsf(model.min_y, -0.305050f, RGF_TEST_EPSILON);
  assert_equalsf(model.min_z, -0.221103f, RGF_TEST_EPSILON);
  assert_equalsf(model.max_x, 0.232135f, RGF_TEST_EPSILON);
  assert_equalsf(model.max_y, 0.116040f, RGF_TEST_EPSILON);
  assert_equalsf(model.max_z, 0.053476f, RGF_TEST_EPSILON);
  assert_equalsf(model.center_x, 0.005490f, RGF_TEST_EPSILON);
  assert_equalsf(model.center_y, -0.094505f, RGF_TEST_EPSILON);
  assert_equalsf(model.center_z, -0.083813f, RGF_TEST_EPSILON);
}

int main(void)
{
  rgf_test_encode_decode();
  rgf_test_encode_to_file();
  rgf_test_decode_from_file();
  rgf_test_parse_obj();

  return 0;
}

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
