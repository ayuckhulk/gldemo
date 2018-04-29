#include "common.h"

void gl_SetError(GLContext *context, GLenum error) {
    if (context->error == GL_NO_ERROR) {
        context->error = error;
    }
}

GLenum gl_GetError(GLContext *context) {
    GLenum error = context->error;
    context->error = GL_NO_ERROR;
    return error;
}

GLenum glGetError(void) {
    return gl_GetError(gl_GetCurrentContext());
}

void glClientActiveTexture(GLenum texture) {
    GLContext *context = gl_GetCurrentContext();
    GLint index = texture - GL_TEXTURE0;

    if (index < 0 || index >= MAX_TEXTURE_UNITS) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }
    context->client_texture_unit = index;
}

void glActiveTexture(GLenum texture) {
    GLContext *context = gl_GetCurrentContext();
    GLint index = texture - GL_TEXTURE0;

    if (index < 0 || index >= MAX_TEXTURE_UNITS) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }
    context->texture_unit = index;
    context->matrix[TEXTURE_MATRIX] = context->TU[index]->matrix;
}

void gl_SetEnabled(GLenum cap, GLboolean enabled) {
    GLContext *context = gl_GetCurrentContext();
    GLint i;
    i = cap - GL_LIGHT0;
    if (i >= 0 && i < MAX_LIGHTS) {
        context->lights[i].enabled = enabled;
    } else {
        switch (cap) {
        case GL_TEXTURE_2D:
            context->TU[context->texture_unit]->enabled = enabled;
            break;
        case GL_ALPHA_TEST:
        	context->alpha_test_enabled = enabled;
        	break;
        case GL_DEPTH_TEST:
            context->depth_test_enabled = enabled;
            break;
        case GL_BLEND:
            context->blend_enabled = enabled;
            break;
        case GL_CULL_FACE:
            context->cull_face_enabled = enabled;
            break;
        case GL_RESCALE_NORMAL:
            context->rescale_normal_enabled = enabled;
            context->update_lighting = GL_TRUE;
            break;
        case GL_NORMALIZE:
            context->normalize_enabled = enabled;
            break;
        case GL_LIGHTING:
            context->lighting_enabled = enabled;
            break;
        case GL_COLOR_MATERIAL:
            context->color_material_enabled = enabled;
            break;
        default:
            gl_SetError(context, GL_INVALID_ENUM);
            return;
        }
    }
}

void glEnable(GLenum cap) {
    gl_SetEnabled(cap, GL_TRUE);
}

void glDisable(GLenum cap) {
    gl_SetEnabled(cap, GL_FALSE);
}

void glGetFixedv(GLenum pname, GLfixed *params) {
    GLContext *context = gl_GetCurrentContext();
    switch (pname) {
    case GL_MODELVIEW_MATRIX:
        memcpy( params,
                MatrixStack_GetCurrent(context->matrix[MODELVIEW_MATRIX])->M,
                16 * sizeof(GLfixed));
        break;
    case GL_PROJECTION_MATRIX:
        memcpy( params,
                MatrixStack_GetCurrent(context->matrix[PROJECTION_MATRIX])->M,
                16 * sizeof(GLfixed));
        break;
    case GL_TEXTURE_MATRIX:
        memcpy( params,
                MatrixStack_GetCurrent(context->matrix[TEXTURE_MATRIX])->M,
                16 * sizeof(GLfixed));
        break;
    default:
        gl_SetError(context, GL_INVALID_ENUM);
        break;
    }
}

void glPixelStorei(GLenum pname, GLint param) {
    GLContext *context = gl_GetCurrentContext();
    switch (pname) {
    case GL_UNPACK_ALIGNMENT:
        if (param != 1 && param != 2 && param != 4 && param != 8) {
            gl_SetError(context, GL_INVALID_VALUE);
            return;
        }
        context->unpack_alignment = param;
        break;
    default:
        gl_SetError(context, GL_INVALID_ENUM);
        break;
    }
}

void glHint(GLenum target, GLenum mode) {
    GLContext *context = gl_GetCurrentContext();
    switch (target) {
    case GL_PERSPECTIVE_CORRECTION_HINT:
        if (mode == GL_FASTEST)
            context->perspective_correction = GL_FALSE;
        else if (mode == GL_NICEST || mode == GL_DONT_CARE)
            context->perspective_correction = GL_TRUE;
        else
            gl_SetError(context, GL_INVALID_ENUM);
        break;
    default:
        gl_SetError(context, GL_INVALID_ENUM);
        break;
    }
}

void glShadeModel (GLenum mode) {
    GLContext *context = gl_GetCurrentContext();

    switch (mode) {
    case GL_SMOOTH:
    case GL_FLAT:
        context->shade_model = mode;
        break;
    default:
        gl_SetError(context, GL_INVALID_ENUM);
        break;
    }
}

void glCullFace(GLenum mode) {
    GLContext *context = gl_GetCurrentContext();
    switch (mode) {
    case GL_FRONT:
    case GL_BACK:
    case GL_FRONT_AND_BACK:
        context->cull_face_mode = mode;
        break;
    default:
        gl_SetError(context, GL_INVALID_ENUM);
        break;
    }
}

void glFrontFace(GLenum mode) {
    GLContext *context = gl_GetCurrentContext();
    switch (mode) {
    case GL_CCW:
    case GL_CW:
        context->front_face = mode;
        break;
    default:
        gl_SetError(context, GL_INVALID_ENUM);
        break;
    }
}

void glBlendFunc(GLenum sfactor, GLenum dfactor) {
    GLContext *context = gl_GetCurrentContext();

    if (sfactor != GL_SRC_ALPHA || dfactor != GL_ONE_MINUS_SRC_ALPHA) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }
}

static inline GLubyte clamp8(int x) {
	if (x > 255) return 255;
	else return x;
}

void glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
    GLContext *context = gl_GetCurrentContext();
    context->current_color[0] = clamp8(fixed_toint(red * 255));
    context->current_color[1] = clamp8(fixed_toint(green * 255));
    context->current_color[2] = clamp8(fixed_toint(blue * 255));
    context->current_color[3] = clamp8(fixed_toint(alpha * 255));
}

void glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz) {
    GLContext *context = gl_GetCurrentContext();
    context->current_normal.p.x = nx;
    context->current_normal.p.y = ny;
    context->current_normal.p.z = nz;
}

PIXEL ColorRGBx(GLclampx red, GLclampx green, GLclampx blue) {
    GLushort r = fixed_toint(red * 255);
    GLushort g = fixed_toint(green * 255);
    GLushort b = fixed_toint(blue * 255);
    return RGB_TO_PIXEL(r, g, b);
}

void glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha) {
    GLContext *context = gl_GetCurrentContext();
    context->clear_color = ColorRGBx(red, green, blue);
}

void glClearDepthx(GLclampx depth) {
    GLContext *context = gl_GetCurrentContext();
    context->clear_depth = depth;
}

void glGetIntegerv (GLenum pname, GLint *params) {
	GLContext *context = gl_GetCurrentContext();

	switch (pname) {
	case GL_SHADE_MODEL:
		*params = context->shade_model;
		break;
	default:
		gl_SetError(context, GL_INVALID_ENUM);
		break;
	}
}

void glAlphaFuncx (GLenum func, GLclampx ref) {
	GLContext *context = gl_GetCurrentContext();

    if (func != GL_NOTEQUAL) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }
	context->alpha_value = toAlpha(fixed_toint(255 * ref));
}

