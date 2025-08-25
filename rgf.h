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
  unsigned long normals_size;
  unsigned long tangents_size;
  unsigned long bitangents_size;
  unsigned long uvs_size; /* Number of floats in the uvs array */
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

  /* New fields to store the original center of the model after parsing */
  float original_center_x;
  float original_center_y;
  float original_center_z;

  float original_max_dim; /* The largest dimension of the model before any scaling. */
  float current_scale;    /* The current scaling factor relative to the original.  */

  float *vertices;   /* The vertex data */
  float *normals;    /* Normals data */
  float *tangents;   /* Point along the U-axis of the texture*/
  float *bitangents; /* Point along the V-axis of the texture. */
  float *uvs;        /* Texture coordinates: size = (vertices_size / 3) * 2 */
  int *indices;      /* Vertex indices data */

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
/* # Utility Vector Functions                                 */
/* ########################################################## */
RGF_API RGF_INLINE float rgf_sqrtf(float f)
{
  float guess = f / 2.0f;

  /* Perform a few iterations of Newton-Raphson to refine the guess */
  guess = 0.5f * (guess + f / guess);
  guess = 0.5f * (guess + f / guess);
  guess = 0.5f * (guess + f / guess);
  guess = 0.5f * (guess + f / guess);
  guess = 0.5f * (guess + f / guess);

  return guess;
}

RGF_API RGF_INLINE void rgf_v3_sub(float *out, float *a, float *b)
{
  out[0] = a[0] - b[0];
  out[1] = a[1] - b[1];
  out[2] = a[2] - b[2];
}

RGF_API RGF_INLINE void rgf_v3_add(float *out, float *a, float *b)
{
  out[0] = a[0] + b[0];
  out[1] = a[1] + b[1];
  out[2] = a[2] + b[2];
}

RGF_API RGF_INLINE void rgf_v3_cross(float *out, float *a, float *b)
{
  out[0] = a[1] * b[2] - a[2] * b[1];
  out[1] = a[2] * b[0] - a[0] * b[2];
  out[2] = a[0] * b[1] - a[1] * b[0];
}

RGF_API RGF_INLINE float rgf_v3_length(float *v)
{
  return rgf_sqrtf((v[0] * v[0] + v[1] * v[1] + v[2] * v[2]));
}

RGF_API RGF_INLINE void rgf_v3_normalize(float *out, float *v)
{
  float len = rgf_v3_length(v);
  if (len > 0.0f)
  {
    float inv_len = 1.0f / len;
    out[0] = v[0] * inv_len;
    out[1] = v[1] * inv_len;
    out[2] = v[2] * inv_len;
  }
  else
  {
    out[0] = out[1] = out[2] = 0.0f;
  }
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
  unsigned long uv_count = 0;
  unsigned long v_index = 0;
  unsigned long uv_index = 0;
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
    else if (obj_binary[i] == 'v' && obj_binary[i + 1] == 't')
    {
      uv_count++;
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
  model->uvs_size = uv_count * 2;

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
    /* -------- Vertex -------- */
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
    /* -------- Texture Coordinate -------- */
    else if (obj_binary[i] == 'v' && obj_binary[i + 1] == 't')
    {
      int consumed = 0;
      i += 3; /* skip "vt " */

      model->uvs[uv_index++] = rgf_atof((char *)(obj_binary + i), &consumed);
      i += (unsigned long)consumed;
      while (rgf_is_space((char)obj_binary[i]))
        i++;

      model->uvs[uv_index++] = rgf_atof((char *)(obj_binary + i), &consumed);
      i += (unsigned long)consumed;
    }
    /* -------- Faces -------- */
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

  /* Store the original center position */
  model->original_center_x = model->center_x;
  model->original_center_y = model->center_y;
  model->original_center_z = model->center_z;

  /* Calculate the initial dimensions and set scale */
  model->original_max_dim = model->max_x - model->min_x;

  if (model->max_y - model->min_y > model->original_max_dim)
  {
    model->original_max_dim = model->max_y - model->min_y;
  }

  if (model->max_z - model->min_z > model->original_max_dim)
  {
    model->original_max_dim = model->max_z - model->min_z;
  }

  model->current_scale = 1.0f;

  return 1;
}

/* ########################################################## */
/* # Geometry manipulation functions                          */
/* ########################################################## */
RGF_API RGF_INLINE void rgf_model_calculate_normals(rgf_model *model)
{
  unsigned long i;
  float v1[3], v2[3], v3[3];
  float edge1[3], edge2[3];
  float face_normal[3];
  int idx1, idx2, idx3;

  /* Initialize all normals to zero */
  for (i = 0; i < model->vertices_size; ++i)
  {
    model->normals[i] = 0.0f;
  }

  /* Iterate through each triangle in the model */
  for (i = 0; i < model->indices_size; i += 3)
  {
    /* Get the indices for the three vertices of the current triangle */
    idx1 = model->indices[i + 0];
    idx2 = model->indices[i + 1];
    idx3 = model->indices[i + 2];

    /* Get the vertex positions */
    v1[0] = model->vertices[idx1 * 3 + 0];
    v1[1] = model->vertices[idx1 * 3 + 1];
    v1[2] = model->vertices[idx1 * 3 + 2];

    v2[0] = model->vertices[idx2 * 3 + 0];
    v2[1] = model->vertices[idx2 * 3 + 1];
    v2[2] = model->vertices[idx2 * 3 + 2];

    v3[0] = model->vertices[idx3 * 3 + 0];
    v3[1] = model->vertices[idx3 * 3 + 1];
    v3[2] = model->vertices[idx3 * 3 + 2];

    /* Calculate the two edge vectors for the triangle */
    rgf_v3_sub(edge1, v2, v1);
    rgf_v3_sub(edge2, v3, v1);

    /* Compute the cross product to get the face normal */
    rgf_v3_cross(face_normal, edge1, edge2);

    /* Add this face normal to the vertex normals of all three vertices */
    rgf_v3_add(&model->normals[idx1 * 3], &model->normals[idx1 * 3], face_normal);
    rgf_v3_add(&model->normals[idx2 * 3], &model->normals[idx2 * 3], face_normal);
    rgf_v3_add(&model->normals[idx3 * 3], &model->normals[idx3 * 3], face_normal);
  }

  /* Normalize each vertex normal to get the final smooth normal */
  for (i = 0; i < model->vertices_size; i += 3)
  {
    rgf_v3_normalize(&model->normals[i], &model->normals[i]);
  }
}

RGF_API RGF_INLINE void rgf_model_calculate_tangents_bitangents(rgf_model *model)
{
  unsigned long i;

  if (!model || !model->vertices || !model->indices || !model->uvs)
  {
    return;
  }

  model->tangents_size = model->vertices_size;
  model->bitangents_size = model->vertices_size;

  /* Initialize to zero */
  for (i = 0; i < model->vertices_size; ++i)
  {
    model->tangents[i] = 0.0f;
    model->bitangents[i] = 0.0f;
  }

  /* Loop through each triangle */
  for (i = 0; i < model->indices_size; i += 3)
  {
    unsigned long j;

    int i1 = model->indices[i + 0];
    int i2 = model->indices[i + 1];
    int i3 = model->indices[i + 2];

    /* Vertex positions */
    float *v0 = &model->vertices[i1 * 3];
    float *v1 = &model->vertices[i2 * 3];
    float *v2 = &model->vertices[i3 * 3];

    /* UV coordinates */
    float *uv0 = &model->uvs[i1 * 2];
    float *uv1 = &model->uvs[i2 * 2];
    float *uv2 = &model->uvs[i3 * 2];

    /* Edges of the triangle : position delta */
    float deltaPos1[3], deltaPos2[3];

    /* UV delta */
    float deltaUV1[2];
    float deltaUV2[2];

    /* Calculate r = 1 / (determinant of UV matrix) */
    float r;

    float tangent[3];
    float bitangent[3];

    deltaUV1[0] = uv1[0] - uv0[0];
    deltaUV1[1] = uv1[1] - uv0[1];

    deltaUV2[0] = uv2[0] - uv0[0];
    deltaUV2[1] = uv2[1] - uv0[1];

    r = deltaUV1[0] * deltaUV2[1] - deltaUV1[1] * deltaUV2[0];
    r = r != 0.0f ? 1.0f / r : 0.0f;

    rgf_v3_sub(deltaPos1, v1, v0);
    rgf_v3_sub(deltaPos2, v2, v0);

    /* Tangent & Bitangent */
    tangent[0] = (deltaPos1[0] * deltaUV2[1] - deltaPos2[0] * deltaUV1[1]) * r;
    tangent[1] = (deltaPos1[1] * deltaUV2[1] - deltaPos2[1] * deltaUV1[1]) * r;
    tangent[2] = (deltaPos1[2] * deltaUV2[1] - deltaPos2[2] * deltaUV1[1]) * r;

    bitangent[0] = (deltaPos2[0] * deltaUV1[0] - deltaPos1[0] * deltaUV2[0]) * r;
    bitangent[1] = (deltaPos2[1] * deltaUV1[0] - deltaPos1[1] * deltaUV2[0]) * r;
    bitangent[2] = (deltaPos2[2] * deltaUV1[0] - deltaPos1[2] * deltaUV2[0]) * r;

    /* Add tangent and bitangent to each vertex of the triangle */
    for (j = 0; j < 3; ++j)
    {
      model->tangents[(model->indices[i + j]) * 3 + 0] += tangent[0];
      model->tangents[(model->indices[i + j]) * 3 + 1] += tangent[1];
      model->tangents[(model->indices[i + j]) * 3 + 2] += tangent[2];

      model->bitangents[(model->indices[i + j]) * 3 + 0] += bitangent[0];
      model->bitangents[(model->indices[i + j]) * 3 + 1] += bitangent[1];
      model->bitangents[(model->indices[i + j]) * 3 + 2] += bitangent[2];
    }
  }

  /* Normalize tangents and bitangents */
  for (i = 0; i < model->vertices_size; i += 3)
  {
    rgf_v3_normalize(&model->tangents[i], &model->tangents[i]);
    rgf_v3_normalize(&model->bitangents[i], &model->bitangents[i]);
  }
}

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

RGF_API RGF_INLINE void rgf_model_center_reset(
    rgf_model *model)
{
  float current_center_x, current_center_y, current_center_z;
  float offset_x, offset_y, offset_z;
  unsigned long i;

  /* Recalculate the current center of the model's bounding box */
  current_center_x = (model->min_x + model->max_x) / 2.0f;
  current_center_y = (model->min_y + model->max_y) / 2.0f;
  current_center_z = (model->min_z + model->max_z) / 2.0f;

  /* Determine the offset needed to move to the original center */
  offset_x = model->original_center_x - current_center_x;
  offset_y = model->original_center_y - current_center_y;
  offset_z = model->original_center_z - current_center_z;

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
  model->center_x = model->original_center_x;
  model->center_y = model->original_center_y;
  model->center_z = model->original_center_z;
}

RGF_API RGF_INLINE void rgf_model_scale(
    rgf_model *model,
    float scale)
{
  unsigned long i;
  float new_scale_factor;

  /* Handle degenerate case of no vertices */
  if (model->original_max_dim <= 0.0f)
  {
    return;
  }

  /* Calculate the desired scaling factor */
  new_scale_factor = scale / model->original_max_dim;

  /* Avoid redundant scaling if already at the desired scale */
  if (new_scale_factor == model->current_scale)
  {
    return;
  }

  /* Calculate the actual scaling factor to apply incrementally */
  new_scale_factor = new_scale_factor / model->current_scale;

  /* Apply the new scaling factor to all vertices */
  for (i = 0; i < model->vertices_size; i++)
  {
    model->vertices[i] *= new_scale_factor;
  }

  /* Update the model's bounding box and scale properties */
  model->min_x *= new_scale_factor;
  model->max_x *= new_scale_factor;
  model->min_y *= new_scale_factor;
  model->max_y *= new_scale_factor;
  model->min_z *= new_scale_factor;
  model->max_z *= new_scale_factor;
  model->center_x *= new_scale_factor;
  model->center_y *= new_scale_factor;
  model->center_z *= new_scale_factor;
  model->current_scale *= new_scale_factor;
}

RGF_API RGF_INLINE void rgf_model_scale_reset(
    rgf_model *model)
{
  unsigned long i;
  float reset_factor;

  /* Handle degenerate cases */
  if (model->original_max_dim <= 0.0f || model->current_scale == 1.0f)
  {
    return;
  }

  /* Calculate the inverse of the current scale to revert to original size */
  reset_factor = 1.0f / model->current_scale;

  /* Apply the reset factor to all vertices */
  for (i = 0; i < model->vertices_size; i++)
  {
    model->vertices[i] *= reset_factor;
  }

  /* Recalculate the bounds from the new vertices */
  model->min_x = model->vertices[0];
  model->max_x = model->vertices[0];
  model->min_y = model->vertices[1];
  model->max_y = model->vertices[1];
  model->min_z = model->vertices[2];
  model->max_z = model->vertices[2];

  for (i = 3; i < model->vertices_size; i += 3)
  {
    if (model->vertices[i] < model->min_x)
      model->min_x = model->vertices[i];
    if (model->vertices[i] > model->max_x)
      model->max_x = model->vertices[i];
    if (model->vertices[i + 1] < model->min_y)
      model->min_y = model->vertices[i + 1];
    if (model->vertices[i + 1] > model->max_y)
      model->max_y = model->vertices[i + 1];
    if (model->vertices[i + 2] < model->min_z)
      model->min_z = model->vertices[i + 2];
    if (model->vertices[i + 2] > model->max_z)
      model->max_z = model->vertices[i + 2];
  }

  /* Update the center and current scale */
  model->center_x = (model->min_x + model->max_x) / 2.0f;
  model->center_y = (model->min_y + model->max_y) / 2.0f;
  model->center_z = (model->min_z + model->max_z) / 2.0f;
  model->current_scale = 1.0f;
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

  unsigned long size_total =
      (unsigned long)(RGF_BINARY_SIZE_HEADER +    /* Header                                      */
                      6 * sizeof(unsigned long) + /* Vertices/Normals/Indices/... sizes          */
                      14 * sizeof(int)            /* Boundaries of geometry (min,max,center,...) */
      );

  /* Depending on which data is set in the rgf model we calculate the total size needed to store */
  if (model->vertices && model->vertices_size > 0)
  {
    size_total += (unsigned long)(model->vertices_size * sizeof(float));
  }

  if (model->normals && model->normals_size > 0)
  {
    size_total += (unsigned long)(model->normals_size * sizeof(float));
  }

  if (model->tangents && model->tangents_size > 0)
  {
    size_total += (unsigned long)(model->tangents_size * sizeof(float));
  }

  if (model->bitangents && model->bitangents_size > 0)
  {
    size_total += (unsigned long)(model->bitangents_size * sizeof(float));
  }

  if (model->uvs && model->uvs_size > 0)
  {
    size_total += (unsigned long)(model->uvs_size * sizeof(float));
  }

  if (model->indices && model->indices_size > 0)
  {
    size_total += (unsigned long)(model->indices_size * sizeof(int));
  }

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
  rgf_binary_memcpy(ptr, &model->normals_size, (unsigned long)sizeof(unsigned long));
  ptr += sizeof(unsigned long);
  rgf_binary_memcpy(ptr, &model->tangents_size, (unsigned long)sizeof(unsigned long));
  ptr += sizeof(unsigned long);
  rgf_binary_memcpy(ptr, &model->bitangents_size, (unsigned long)sizeof(unsigned long));
  ptr += sizeof(unsigned long);
  rgf_binary_memcpy(ptr, &model->uvs_size, (unsigned long)sizeof(unsigned long));
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

  rgf_binary_memcpy(ptr, &model->original_center_x, 4);
  ptr += 4;
  rgf_binary_memcpy(ptr, &model->original_center_y, 4);
  ptr += 4;
  rgf_binary_memcpy(ptr, &model->original_center_z, 4);
  ptr += 4;

  rgf_binary_memcpy(ptr, &model->original_max_dim, 4);
  ptr += 4;
  rgf_binary_memcpy(ptr, &model->current_scale, 4);
  ptr += 4;

  /* Depending on which data is set in the rgf model we copy the data into the binary buffer */
  if (model->vertices && model->vertices_size > 0)
  {
    unsigned long size = model->vertices_size * sizeof(float);
    rgf_binary_memcpy(ptr, model->vertices, size);
    ptr += size;
  }

  if (model->normals && model->normals_size > 0)
  {
    unsigned long size = model->normals_size * sizeof(float);
    rgf_binary_memcpy(ptr, model->normals, size);
    ptr += size;
  }

  if (model->tangents && model->tangents_size > 0)
  {
    unsigned long size = model->tangents_size * sizeof(float);
    rgf_binary_memcpy(ptr, model->tangents, size);
    ptr += size;
  }

  if (model->bitangents && model->bitangents_size > 0)
  {
    unsigned long size = model->bitangents_size * sizeof(float);
    rgf_binary_memcpy(ptr, model->bitangents, size);
    ptr += size;
  }

  if (model->uvs && model->uvs_size > 0)
  {
    unsigned long size = model->uvs_size * sizeof(float);
    rgf_binary_memcpy(ptr, model->uvs, size);
    ptr += size;
  }

  if (model->indices && model->indices_size > 0)
  {
    unsigned long size = model->indices_size * sizeof(int);
    rgf_binary_memcpy(ptr, model->indices, size);
    ptr += size;
  }

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
  model->normals_size = rgf_binary_read_ul(binary_ptr);
  binary_ptr += sizeof(unsigned long);
  model->tangents_size = rgf_binary_read_ul(binary_ptr);
  binary_ptr += sizeof(unsigned long);
  model->bitangents_size = rgf_binary_read_ul(binary_ptr);
  binary_ptr += sizeof(unsigned long);
  model->uvs_size = rgf_binary_read_ul(binary_ptr);
  binary_ptr += sizeof(unsigned long);
  model->indices_size = rgf_binary_read_ul(binary_ptr);
  binary_ptr += sizeof(unsigned long);

  size_total =
      (unsigned long)(RGF_BINARY_SIZE_HEADER +    /* Header                                  */
                      6 * sizeof(unsigned long) + /* Vertices/Normals/Indices count           */
                      14 * sizeof(float)          /* Boundaries of geometry (min,max,center) */
      );

  /* Depending on which data is set in the rgf model we calculate the total size needed to store */
  if (model->vertices && model->vertices_size > 0)
  {
    size_total += (unsigned long)(model->vertices_size * sizeof(float));
  }

  if (model->normals && model->normals_size > 0)
  {
    size_total += (unsigned long)(model->normals_size * sizeof(float));
  }

  if (model->tangents && model->tangents_size > 0)
  {
    size_total += (unsigned long)(model->tangents_size * sizeof(float));
  }

  if (model->bitangents && model->bitangents_size > 0)
  {
    size_total += (unsigned long)(model->bitangents_size * sizeof(float));
  }

  if (model->uvs && model->uvs_size > 0)
  {
    size_total += (unsigned long)(model->uvs_size * sizeof(float));
  }

  if (model->indices && model->indices_size > 0)
  {
    size_total += (unsigned long)(model->indices_size * sizeof(int));
  }

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

  model->original_center_x = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;
  model->original_center_y = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;
  model->original_center_z = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;

  model->original_max_dim = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;
  model->current_scale = rgf_binary_read_float(binary_ptr);
  binary_ptr += 4;

  if (model->vertices_size > 0)
  {
    model->vertices = (float *)binary_ptr;
    binary_ptr += model->vertices_size * sizeof(float);
  }

  if (model->normals_size > 0)
  {
    model->normals = (float *)binary_ptr;
    binary_ptr += model->normals_size * sizeof(float);
  }

  if (model->tangents_size > 0)
  {
    model->tangents = (float *)binary_ptr;
    binary_ptr += model->tangents_size * sizeof(float);
  }

  if (model->bitangents_size > 0)
  {
    model->bitangents = (float *)binary_ptr;
    binary_ptr += model->bitangents_size * sizeof(float);
  }

  if (model->uvs_size > 0)
  {
    model->uvs = (float *)binary_ptr;
    binary_ptr += model->uvs_size * sizeof(float);
  }

  if (model->indices_size > 0)
  {
    model->indices = (int *)binary_ptr;
    binary_ptr += model->indices_size * sizeof(int);
  }

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
