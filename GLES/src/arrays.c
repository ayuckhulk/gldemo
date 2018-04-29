#include "common.h"

void Array_Init(Array *array) {
    array->enabled = GL_FALSE;
    array->base = NULL;
}

void Array_Set(Array *array, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
    array->size = size;
    array->type = type;
    array->base = (GLubyte *) pointer;

    array->current = array->base;

    if (stride == 0) {
        switch(type) {
        case GL_FIXED : array->stride = array->size * sizeof (GLfixed); break;
        case GL_SHORT : array->stride = array->size * sizeof (GLshort); break;
        case GL_UNSIGNED_BYTE : array->stride = array->size * sizeof (GLubyte); break;
        case GL_FLOAT : array->stride = array->size * sizeof (GLfloat); break;
        }
    } else array->stride = stride;
}


static inline GLubyte *Array_SetIndex(Array *array, GLuint index) {
    return (array->current = array->base + array->stride * index);
}

static inline GLubyte *Array_Next(Array *array) {
    return (array->current += array->stride);
}

void Array_GetColor(Array *array, GLubyte *c) {
    switch (array->type) {
        case GL_UNSIGNED_BYTE :
            memcpy(c, array->current, array->size * sizeof(GLubyte));
            break;
        case GL_FIXED:
            c[0] = fixed_toint(((GLfixed *) array->current)[0] * 255);
            c[1] = fixed_toint(((GLfixed *) array->current)[1] * 255);
            c[2] = fixed_toint(((GLfixed *) array->current)[2] * 255);
            c[3] = fixed_toint(((GLfixed *) array->current)[3] * 255);
            break;
    }
}

void Array_GetVector(Array *array, vec4 *v) {
    GLuint i;
    v->p.z = fixed_int(0);
    v->p.w = fixed_int(1);
    switch (array->type) {
        case GL_SHORT :
            for (i = 0;i < array->size;i++) {
                v->xyzw[i] = fixed_int(((GLshort *)array->current)[i]);
            }
             break;
        case GL_FIXED:
            for (i = 0;i < array->size;i++)
                v->xyzw[i] = ((GLfixed *)array->current)[i];
            break;
    }
}

static inline void GetColor(GLContext *context, GLubyte *color) {
    Array *array = &context->color;
    if (array->enabled) Array_GetColor(array, color);
    else memcpy(color, context->current_color, 4 * sizeof(GLubyte));
}

static inline void GetTexCoord(TextureUnit *TU, vec4 *texcoord) {
    vec4 buf;
    vec4 tmp;
    vec4 *v;
    mat4 *m = (TU->matrix->identity ? NULL : MatrixStack_GetCurrent(TU->matrix));
    Array *array = &TU->texcoord;

    if (array->enabled) {
        v = &buf;
        if (m) {
            Array_GetVector(array, &tmp);
            mat4_mul_vec4(v, m, &tmp);
        } else{
            Array_GetVector(array, v);
        }
    } else {
        if (m) {
            v = &buf;
            mat4_mul_vec4(v, m, &(TU->current_texcoord));
        } else {
            v = &TU->current_texcoord;
        }
    }

    if (v->p.w != fixed_int(0) && v->p.w != fixed_int(1)) {
        texcoord->p.x = fixed_div(TU->scales * v->p.x, v->p.w);
        texcoord->p.y = fixed_div(TU->scalet * v->p.y, v->p.w);
    } else {
        texcoord->p.x = TU->scales * v->p.x;
        texcoord->p.y = TU->scalet * v->p.y;
    }
}

static inline GLfixed ConvertToNormalx(GLshort p) {
    if (p == 32767) return fixed_int(1);
    if (p == -32768) return - fixed_int(1);
    return ((((GLfixed)p) << 1) + 1);
}

void Array_GetNormal(Array *array, vec3 *n) {
    GLuint i;
    switch (array->type) {
    case GL_SHORT :
        for (i = 0;i < array->size;i++)
            n->xyz[i] = ConvertToNormalx(((GLshort *)array->current)[i]);
        break;
    case GL_FIXED:
        for (i = 0;i < array->size;i++)
            n->xyz[i] = ((GLfixed *)array->current)[i];
        break;
    }
}

static inline void GetNormal(GLContext *context, vec3 *normal) {
    vec3 tmp;
    vec3 buf;
    vec3 *n;
    Array *array = &context->normal;

    if (context->normalize_enabled || context->rescale_normal_enabled) n = &buf;
    else n = normal;

    if (array->enabled) {
        Array_GetNormal(array, &tmp);
        vec3_mul_mat3( n, &tmp, &(context->normal_transform) );
    } else {
        vec3_mul_mat3( n, &(context->current_normal), &(context->normal_transform) );
    }

    if (context->normalize_enabled) {
        vec3_normalize(normal, n);
        //printf("n = "); Vec3Print(n);
        //printf("normal = "); Vec3Print(normal);
    }
    else if (context->rescale_normal_enabled) vec3_scale(normal, n, context->rescale_normal);
}

void gl_SetEnabledClientState(GLenum array, GLboolean enabled) {
    GLContext *context = gl_GetCurrentContext();
    switch(array) {
        case GL_VERTEX_ARRAY :
          context->vertex.enabled = enabled;
          break;
        case GL_NORMAL_ARRAY :
            context->normal.enabled = enabled;
            break;
        case GL_COLOR_ARRAY :
          context->color.enabled = enabled;
          break;
        case GL_TEXTURE_COORD_ARRAY :
          context->TU[context->client_texture_unit]->texcoord.enabled = enabled;
          break;
        default :
          gl_SetError(context, GL_INVALID_ENUM);
          return;
    }
}

void glDisableClientState (GLenum array) {
    gl_SetEnabledClientState(array, GL_FALSE);
}

void glEnableClientState (GLenum array) {
    gl_SetEnabledClientState(array, GL_TRUE);
}

void glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
    GLContext *context = gl_GetCurrentContext();

    if ((size != 2 && size != 3 && size != 4) || stride < 0) {
        gl_SetError(context, GL_INVALID_VALUE);
        return;
    }

    if (type != GL_FIXED && type != GL_SHORT) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    Array_Set(&context->TU[context->client_texture_unit]->texcoord, size, type, stride, pointer);
}

void glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
    GLContext *context = gl_GetCurrentContext();

    if (size != 4 || stride < 0) {
        gl_SetError(context, GL_INVALID_VALUE);
        return;
    }

    if (type != GL_UNSIGNED_BYTE && type != GL_FIXED) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    Array_Set(&context->color, size, type, stride, pointer);
}

void glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
    GLContext *context = gl_GetCurrentContext();

    if ((size != 2 && size != 3 && size != 4) || stride < 0) {
        gl_SetError(context, GL_INVALID_VALUE);
        return;
    }

    if (type != GL_FIXED && type != GL_SHORT) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    Array_Set(&context->vertex, size, type, stride, pointer);
}

void glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer) {
    GLContext *context = gl_GetCurrentContext();

    if (stride < 0) {
        gl_SetError(context, GL_INVALID_VALUE);
        return;
    }

    if (type != GL_FIXED && type != GL_SHORT) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    Array_Set(&context->normal, 3, type, stride, pointer);
}

void InitTransformAndLighting() {
    GLContext *context = gl_GetCurrentContext();
    MatrixStack *modelview = context->matrix[MODELVIEW_MATRIX];
    MatrixStack *projection = context->matrix[PROJECTION_MATRIX];
    GLuint i;

    if (MatrixStack_IsModified(modelview) || MatrixStack_IsModified(projection)) {
        MatrixStack_Mult(projection, modelview, &(context->transform_matrix));
    }

    if (context->lighting_enabled && (MatrixStack_IsModified(modelview) || context->update_lighting)) {
        mat3 temp;

        mat3_from_mat4(&temp, MatrixStack_GetCurrent(modelview));
        mat3_inverse(&(context->normal_transform), &temp);

        if (context->rescale_normal_enabled) {
            mat3 *m = &(context->normal_transform);
            GLfixed s = fixed_sqrt(fixed_mul(m->M[2], m->M[2]) + fixed_mul(m->M[5], m->M[5]) + fixed_mul(m->M[8], m->M[8]));
            if (s != fixed_int(0)) context->rescale_normal = fixed_div(fixed_int(1), s);
            else context->rescale_normal = fixed_int(1);
        }

        context->update_lighting = GL_FALSE;
    }
    if (!context->lighting_enabled && modelview->modified) context->update_lighting = GL_TRUE;

    context->matrix[MODELVIEW_MATRIX]->modified = GL_FALSE;
    context->matrix[PROJECTION_MATRIX]->modified = GL_FALSE;

    for (i = 0;i < MAX_TEXTURE_UNITS;i++)
        if (context->TU[i]->enabled) {
            context->TU[i]->scales = context->TU[i]->texture->lod[0].width - 1;
            context->TU[i]->scalet = context->TU[i]->texture->lod[0].height - 1;
        }
}

static inline void gl_ProcessIndex(GLuint index) {
    GLContext *context = gl_GetCurrentContext();
    RenderingBuffer *buffer = &context->rendering_buffer;
    Vertex *vertex = RenderingBuffer_CurrentVertex(buffer);
    vec4 v;
    GLuint i;

    Array_SetIndex(&context->vertex, index);
    if (context->color.enabled) Array_SetIndex(&context->color, index);
    if (context->normal.enabled) Array_SetIndex(&context->normal, index);
    for (i = 0;i < MAX_TEXTURE_UNITS;i++)
        if (context->TU[i]->enabled)
            Array_SetIndex(&context->TU[i]->texcoord, index);

    GetColor(context, vertex->color);

    Array_GetVector( &(context->vertex), &v );
    mat4_mul_vec4( &(vertex->v), &(context->transform_matrix), &v );

    if (context->lighting_enabled && !buffer->data_locked) {
        mat4_mul_vec4( &(RenderingBuffer_CurrentVertexData(buffer)->v), MatrixStack_GetCurrent(context->matrix[MODELVIEW_MATRIX]), &v);
        GetNormal(context, &(RenderingBuffer_CurrentVertexData(buffer)->n));
    }

    for (i = 0;i < MAX_TEXTURE_UNITS;i++)
        if (context->TU[i]->enabled)
            GetTexCoord(context->TU[i], vertex->t + i);

    RenderingBuffer_ProcessVertex(buffer);
}

void glDrawArrays (GLenum mode, GLint first, GLsizei count) {
    GLContext *context = gl_GetCurrentContext();
    GLint i;

    if (count < 0) {
        gl_SetError(context, GL_INVALID_VALUE);
        return;
    }

    if (mode != GL_TRIANGLES && mode != GL_TRIANGLE_FAN && mode != GL_TRIANGLE_STRIP) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    if (!context->vertex.enabled) return;

    InitTransformAndLighting();

    RenderingBuffer_Begin(&context->rendering_buffer, mode);

    for (i = 0;i < count;i++) gl_ProcessIndex(first + i);

    RenderingBuffer_End(&context->rendering_buffer);
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) {
    GLContext *context = gl_GetCurrentContext();
    GLint i;

    if (count < 0) {
        gl_SetError(context, GL_INVALID_VALUE);
        return;
    }

    if (mode != GL_TRIANGLES && mode != GL_TRIANGLE_FAN && mode != GL_TRIANGLE_STRIP) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT) {
        gl_SetError(context, GL_INVALID_VALUE);
        return;
    }

    if (!context->vertex.enabled) return;

    InitTransformAndLighting();

    RenderingBuffer_Begin(&context->rendering_buffer, mode);

    switch (type) {
        case GL_UNSIGNED_BYTE:
            for (i = 0;i < count;i++) gl_ProcessIndex(((GLubyte *)indices)[i]);
            break;
        case GL_UNSIGNED_SHORT:
            for (i = 0;i < count;i++) gl_ProcessIndex(((GLushort *)indices)[i]);
            break;
     }

    RenderingBuffer_End(&context->rendering_buffer);
}

