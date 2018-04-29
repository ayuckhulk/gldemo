#include "common.h"

TextureUnit *TextureUnit_Create(void) {
    TextureUnit *unit;

    unit = (TextureUnit *) malloc(sizeof(TextureUnit));

    if (unit == NULL) return NULL;

    unit->matrix = MatrixStack_Create(MAX_TEXTURE_STACK_DEPTH);
    if (unit->matrix == NULL) {
        free(unit);
        return NULL;
    }
    unit->enabled = GL_FALSE;
    unit->texture = gl_GetCurrentContext()->textures[0];
    Array_Init(&unit->texcoord);
    unit->env_mode = GL_MODULATE;

    return unit;
}

void TextureUnit_Destroy(TextureUnit *unit) {
    if (unit == NULL) return;
    if (unit->matrix) MatrixStack_Destroy(unit->matrix);
    free(unit);
}

Texture *Texture_Create(GLuint id) {
    Texture *texture;
    int i;

    texture = (Texture *) malloc(sizeof(Texture));
    if (texture == NULL) return NULL;

    texture->id = id;
    texture->prev = NULL;
    texture->next = NULL;

    for (i = 0;i < MAX_LOD;i++) {
        texture->lod[i].texels = NULL;
        texture->lod[i].alpha = NULL;
        texture->lod[i].width = 0;
        texture->lod[i].height = 0;
        texture->lod[i].s_mask = 0;
        texture->lod[i].t_mask = 0;
        texture->lod[i].t_shift = 0;
    }

    texture->max_lod = 0;

    texture->min_filter = GL_NEAREST;
    texture->mag_filter = GL_NEAREST;

    return texture;
}

GLboolean Texture_IsComplete(Texture *texture) {
    int i;
    int w, h;

    w = texture->lod[0].width;
    h = texture->lod[0].height;

    if (w == 0 || h == 0) return GL_FALSE;

    if (texture->min_filter != GL_NEAREST_MIPMAP_NEAREST) return GL_TRUE;

    w >>= 1;
    h >>= 1;
    for(i = 1;i <= texture->max_lod;i++) {
        if (texture->lod[i].width != w) return GL_FALSE;
        if (texture->lod[i].height != h) return GL_FALSE;

        if (texture->lod[i].texels == NULL) return GL_FALSE;

        w >>= 1;
        h >>= 1;
        if (w == 0) w = 1;
        if (h == 0) h = 1;
    }
    return GL_TRUE;
}

void Texture_Destroy(Texture *texture) {
    int i;
    if (texture == NULL) return;
    for (i = 0;i < MAX_LOD;i++) {
        if (texture->lod[i].texels) free(texture->lod[i].texels);
        if (texture->lod[i].alpha) free(texture->lod[i].alpha);
    }
    free(texture);
}

inline int log2i(int i) {
    int result = 0;
    while (i >>= 1) result++;
    return result;
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    GLContext *context = gl_GetCurrentContext();
    TextureArray *current;
    Texture *texture;
    PIXEL *data;
    GLubyte *alpha;
    GLubyte *p;
    GLint i, n;

    if (target != GL_TEXTURE_2D) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    if (border != 0 || width < 0 || height < 0 || level < 0) {
        gl_SetError(context, GL_INVALID_VALUE);
        return;
    }

    if (width > MAX_TEXTURE_SIZE || height > MAX_TEXTURE_SIZE || (width & (width - 1)) || (height & (height - 1))) {
        gl_SetError(context, GL_INVALID_VALUE);
        return;
    }

    texture = context->TU[context->texture_unit]->texture;

    if (level > texture->max_lod) {
        gl_SetError(context, GL_INVALID_VALUE);
        return;
    }

    current = &(texture->lod[level]);

    n = width * height;

    // Disable texture
    if (n == 0) {
        if (current->texels) free(current->texels);
        if (current->alpha) free(current->alpha);
        current->texels = NULL;
        current->width = 0;
        current->height = 0;
        return;
    }

    data = (PIXEL *) malloc(n * sizeof(PIXEL));
    if (data == NULL) {
        gl_SetError(context, GL_OUT_OF_MEMORY);
        return;
    }

    alpha = (GLubyte *) malloc(n * sizeof(GLubyte));
    if (alpha == NULL) {
        free(data);
        gl_SetError(context, GL_OUT_OF_MEMORY);
        return;
    }

    if (current->texels) free(current->texels);
    if (current->alpha) free(current->alpha);

    if (level == 0) {
        if (width > height) texture->max_lod = log2i(width);
        else texture->max_lod = log2i(height);
    }

    if (pixels) {
        p = (GLubyte *) pixels;


        switch(format) {
            case GL_ALPHA:
                for (i = 0; i < n;i++) {
                    data[i] = RGB_TO_PIXEL(0, 0, 0);
                    alpha[i] = toAlpha(p[0]);
                    p++;
                }
                break;
            case GL_LUMINANCE:
                for (i = 0; i < n;i++) {
                    data[i] = RGB_TO_PIXEL(p[0], p[0], p[0]);
                    alpha[i] = toAlpha(255);
                    p++;
                }
                break;
            case GL_LUMINANCE_ALPHA:
                for (i = 0; i < n;i++) {
                    data[i] = RGB_TO_PIXEL(p[0], p[0], p[0]);
                    alpha[i] = toAlpha(p[1]);
                    p += 2;
                }
                break;
            case GL_RGB:
                for (i = 0; i < n;i++) {
                    data[i] = RGB_TO_PIXEL(p[0], p[1], p[2]);
                    alpha[i] = toAlpha(255);
                    p += 3;
                }
                break;
            case GL_RGBA:
                for (i = 0; i < n;i++) {
                    data[i] = RGB_TO_PIXEL(p[0], p[1], p[2]);
                    alpha[i] = toAlpha(p[3]);
                    p += 4;
                }
                break;
        }
    }
    current->texels = data;
    current->alpha = alpha;
    current->width = width;
    current->height = height;
    current->s_mask = (width - 1) << 16;
    current->t_mask = (height - 1) << 16;
    current->t_shift = 16 - log2i(width);

}

void glTexEnvx(GLenum target, GLenum pname, GLfixed param) {
	glTexEnvi(target, pname, (GLfixed)param);
}

void glTexEnvi(GLenum target, GLenum pname, GLint param) {
    GLContext *context = gl_GetCurrentContext();

    if (target != GL_TEXTURE_ENV || pname != GL_TEXTURE_ENV_MODE) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    switch(param) {
        case GL_REPLACE : context->TU[context->texture_unit]->env_mode = GL_REPLACE; break;
        case GL_MODULATE : context->TU[context->texture_unit]->env_mode = GL_MODULATE; break;
        //case GL_DECAL : context->TU[context->texture_unit]->env_mode = GL_DECAL; break;
        default : gl_SetError(context, GL_INVALID_VALUE); return;
    }
}

void glTexParameterx (GLenum target, GLenum pname, GLfixed param) {
	glTexParameteri(target, pname, (GLfixed)param);
}

void glTexParameteri (GLenum target, GLenum pname, GLint param) {
    GLContext *context = gl_GetCurrentContext();

    if (target != GL_TEXTURE_2D) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    switch(pname) {
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
            if (param != GL_REPEAT) {
                gl_SetError(context, GL_INVALID_VALUE);
                return;
            }
            break;
        case GL_TEXTURE_MIN_FILTER:
            if (param == GL_NEAREST || param == GL_NEAREST_MIPMAP_NEAREST) {
                context->TU[context->texture_unit]->texture->min_filter = param;
            } else {
                gl_SetError(context, GL_INVALID_VALUE);
                return;
            }
            break;
        case GL_TEXTURE_MAG_FILTER:
            if (param != GL_NEAREST) {
                gl_SetError(context, GL_INVALID_VALUE);
                return;
            }
            break;
        default:
            gl_SetError(context, GL_INVALID_ENUM);
            break;
    }
}

void glBindTexture (GLenum target, GLuint texture) {
    GLContext *context = gl_GetCurrentContext();
    int index = (texture % (MAX_TEXTURES - 1)) + 1;
    Texture *texture_object;

    if (target != GL_TEXTURE_2D) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    texture_object = context->textures[index];
    
    while (texture_object) {
        if (texture_object->id == texture) {
            context->TU[context->texture_unit]->texture = texture_object;
            return;
        } else if (texture_object->next == NULL) {
            texture_object->next = Texture_Create(texture);
            context->TU[context->texture_unit]->texture = texture_object->next;
            texture_object->next->prev = texture_object;
            return;
        } else texture_object = texture_object->next;
    }

    texture_object = Texture_Create(texture);
    context->textures[index] = texture_object;
    context->TU[context->texture_unit]->texture = texture_object;
}

Texture *gl_FindTexture(GLContext *context, GLuint id) {
    int index = (id % (MAX_TEXTURES - 1)) + 1;
    Texture *texture_object = context->textures[index];
    
    while (texture_object) {
        if (texture_object->id == id) return texture_object;
        else texture_object = texture_object->next;
    }

    return NULL;
}

void glGenTextures (GLsizei n, GLuint *textures) {
    GLContext *context = gl_GetCurrentContext();
    GLuint i, id;
    GLuint max = 0;
   
    if (n < 0) {
        gl_SetError(context, GL_INVALID_VALUE);
        return;
    }

    max = ~max;
    i = 0;

    for (id = 1;id <= max;id++) {
        if (!gl_FindTexture(context, id)) {
            textures[i] = id;
            i++;
            if (i >= n) break;
        }
    }
}

void glDeleteTextures (GLsizei n, const GLuint *textures) {
    GLContext *context = gl_GetCurrentContext();
    GLuint i, j;

    if (n < 0) {
        gl_SetError(context, GL_INVALID_VALUE);
        return;
    }

    for (i = 0;i < n;i++) {
        Texture *texture = gl_FindTexture(context, textures[i]);
        if (texture) {
            for (j = 0;j < MAX_TEXTURE_UNITS;j++)
                if (context->TU[j]->texture == texture) context->TU[j]->texture = context->textures[0];
            
            if (texture->prev) {
                texture->prev->next = texture->next;
                texture->next->prev = texture->prev;
            } else {
                context->textures[(texture->id % (MAX_TEXTURES - 1)) + 1] = texture->next;
                if (texture->next) texture->next->prev = NULL;
            }

            Texture_Destroy(texture);
        }
    }
}

void glMultiTexCoord4x (GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q) {
    GLContext *context = gl_GetCurrentContext();
    vec4 *v;
    GLint i;

    i = target - GL_TEXTURE0;
    if (i < 0 || i >= MAX_TEXTURE_UNITS) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    v = &(context->TU[i]->current_texcoord);
    v->xyzw[0] = s;
    v->xyzw[1] = t;
    v->xyzw[2] = r;
    v->xyzw[3] = q;
}
