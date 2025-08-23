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
  rgf_binary_encode(binary_buffer, BINARY_BUFFER_CAPACITY, &binary_buffer_size, &model);

  /* ########################################################## */
  /* # Decode from binary                                       */
  /* ########################################################## */
  rgf_binary_decode(
      binary_buffer,
      binary_buffer_size,
      &binary_model);

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
  rgf_binary_encode(binary_buffer, BINARY_BUFFER_CAPACITY, &binary_buffer_size, &model);

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

  rgf_binary_decode(binary_buffer, binary_buffer_size, &model);

  assert(model.vertices_size == 2);
  assert(model.indices_size == 2);

  assert_equalsf(model.vertices[0], 1.0f, RGF_TEST_EPSILON);
  assert_equalsf(model.vertices[1], 0.0f, RGF_TEST_EPSILON);
  assert(model.indices[0] == 0);
  assert(model.indices[1] == 1);
}

int main(void)
{
  rgf_test_encode_decode();
  rgf_test_encode_to_file();
  rgf_test_decode_from_file();

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
