#include "common.h"

static GLContext *gl_current_context = NULL;

GLContext *gl_GetCurrentContext(void) {
    return gl_current_context;
}

//void gl_BuildDivTable(GLContext *context) {
//    GLint i;
//    context->div_table = (GLfixed*) malloc((1 << 16) * sizeof(GLfixed));
//    for (i = 1;i < (1 << 16);i++) context->div_table[i] = fixed_div(fixed_int(1), i);
//}

GLenum gl_CreateContext(void *pixels, GLsizei width, GLsizei height, GLsizei pitch) {
    GLint i;

    gl_current_context = (GLContext *) malloc(sizeof(GLContext));
    if (gl_current_context == NULL) return GL_OUT_OF_MEMORY;

    gl_current_context->zbuffer = (GLfixed *) malloc(sizeof(GLfixed) * width * height);

    if (gl_current_context->zbuffer == NULL) {
        free(gl_current_context);
        gl_current_context = NULL;
        return GL_OUT_OF_MEMORY;
    }

    gl_current_context->error = GL_NO_ERROR;

    gl_current_context->framebuffer = (PIXEL *) pixels;
    gl_current_context->width = width;
    gl_current_context->height = height;
    gl_current_context->pitch = pitch;
    gl_current_context->perspective_correction = GL_TRUE;

    gl_current_context->clear_color = 0;
    gl_current_context->clear_depth = fixed_int(1);
    gl_current_context->znear = fixed_int(0);
    gl_current_context->zfar = fixed_int(1);
    gl_current_context->depth_test_enabled = GL_FALSE;

    glViewport(0, 0, width, height);

    gl_current_context->cull_face_enabled = GL_FALSE;
    gl_current_context->cull_face_mode = GL_BACK;
    gl_current_context->front_face = GL_CCW;

    gl_current_context->alpha_test_enabled = GL_FALSE;
    gl_current_context->alpha_value = 0;

    Array_Init(&gl_current_context->vertex);
    Array_Init(&gl_current_context->color);
    Array_Init(&gl_current_context->normal);

    for (i = 0;i < MAX_TEXTURE_UNITS;i++) {
        gl_current_context->TU[i] = TextureUnit_Create();
    }

    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);

    gl_current_context->matrix[MODELVIEW_MATRIX] = MatrixStack_Create(MAX_MODELVIEW_STACK_DEPTH);
    gl_current_context->matrix[PROJECTION_MATRIX] = MatrixStack_Create(MAX_PROJECTION_STACK_DEPTH);
    gl_current_context->matrix_mode = MODELVIEW_MATRIX;

    gl_current_context->blend_enabled = GL_FALSE;

    gl_current_context->textures[0] = Texture_Create(0);
    for (i = 1;i < MAX_TEXTURES;i++) gl_current_context->textures[i] = NULL;

    gl_current_context->unpack_alignment = 4;

    gl_current_context->normalize_enabled = GL_FALSE;
    gl_current_context->rescale_normal_enabled = GL_FALSE;
    gl_current_context->color_material_enabled = GL_FALSE;

    gl_current_context->update_lighting = GL_TRUE;

    glShadeModel(GL_SMOOTH);
    glColor4x(fixed_int(1), fixed_int(1), fixed_int(1), fixed_int(1));
    glNormal3x(fixed_int(0), fixed_int(0), fixed_int(1));

    gl_current_context->lighting_enabled = GL_FALSE;

    Lights_Init(gl_current_context->lights);

    rgba_set(gl_current_context->acm, fixed_float(0.2f), fixed_float(0.2f), fixed_float(0.2f), fixed_float(1.0f));
    rgba_set(gl_current_context->dcm, fixed_float(0.8f), fixed_float(0.8f), fixed_float(0.8f), fixed_float(1.0f));
    rgba_set(gl_current_context->scm, fixed_float(0.0f), fixed_float(0.0f), fixed_float(0.0f), fixed_float(1.0f));
    rgba_set(gl_current_context->ecm, fixed_float(0.0f), fixed_float(0.0f), fixed_float(0.0f), fixed_float(1.0f));
    gl_current_context->srm = fixed_float(0.0f);
    rgba_set(gl_current_context->acs, fixed_float(0.2f), fixed_float(0.2f), fixed_float(0.2f), fixed_float(1.0f));
    gl_current_context->tbs = GL_FALSE;

    //gl_BuildDivTable(gl_current_context);
    //gl_current_context->div_table = NULL;

    return GL_NO_ERROR;
}

void gl_DestroyContext(void) {
    GLint i;

    free(gl_current_context->zbuffer);

    //if (gl_current_context->div_table) free(gl_current_context->div_table);

    for (i = 0;i < MAX_TEXTURES;i++) {
        Texture *texture = gl_current_context->textures[i];
        while (texture) {
             Texture *t = texture->next;
             Texture_Destroy(texture);
             texture = t;
        }
    }

    for (i = 0;i < MAX_TEXTURE_UNITS;i++) {
        TextureUnit_Destroy(gl_current_context->TU[i]);
    }

    free(gl_current_context);
    gl_current_context = NULL;
}

void glScissor (GLint x, GLint y, GLsizei width, GLsizei height) {
    GLContext *context = gl_GetCurrentContext();

    context->xmin = x;
    context->xmax = x + width - 1;
    context->ymin = context->height - y - height;
    context->ymax = context->height - y - 1;
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    GLContext *context = gl_GetCurrentContext();
    GLint t;

    context->view_x = x;
    context->view_y = y;
    context->view_width = width;
    context->view_height = height;

    if (x >= 0) context->xmin = x;
    else context->xmin = 0;

    t = x + width - 1;
    if (t < context->width) context->xmax = t;
    else context->xmax = context->width - 1;

    t = context->height - y - height;
    if (t >= 0) context->ymin = t;
    else context->ymin = 0;

    t = context->height - y - 1;
    if (t < context->height) context->ymax = t;
    else context->ymax = context->height - 1;
}

void glClear(GLbitfield mask) {
    GLContext *context = gl_GetCurrentContext();

    if (mask & GL_COLOR_BUFFER_BIT) {
        register uint32_t *framebuffer;
        register uint32_t clear_color;
        register uint32_t n = (context->pitch * context->height) >> 2;

        clear_color = context->clear_color;
        clear_color <<= 16;
        clear_color |= context->clear_color;
        framebuffer = (uint32_t *)context->framebuffer;
        while (n--) *framebuffer++ = clear_color;
    }

    if (mask & GL_DEPTH_BUFFER_BIT) {
        register uint32_t *zbuffer;
        register uint32_t clear_depth;
        register uint32_t n = (context->width * context->height);// >> 1;

        clear_depth = ZBUFFER_SCALE*context->clear_depth;
//        if (context->clear_depth & 0xFFFF0000) clear_depth = 0xFFFFFFFF;
//        else {
//            clear_depth = context->clear_depth;
//            clear_depth <<= 16;
//            clear_depth |= context->clear_depth;
//        }
        //clear_depth = 0xFFFFFFFF;
        zbuffer = (uint32_t *)context->zbuffer;

        while (n--) *zbuffer++ = clear_depth;
    }
  
}

void glFinish (void) {
}

void glFlush (void) {
}

