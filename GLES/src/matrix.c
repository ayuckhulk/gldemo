#include <math.h>
#include "common.h"

void glMatrixMode(GLenum mode) {
    GLContext *context = gl_GetCurrentContext();

    switch(mode) {
        case GL_MODELVIEW :
          context->matrix_mode = MODELVIEW_MATRIX;
          break;
        case GL_PROJECTION :
          context->matrix_mode = PROJECTION_MATRIX;
          break;
        case GL_TEXTURE :
          context->matrix_mode = TEXTURE_MATRIX;
          break;
        default :
          gl_SetError(context, GL_INVALID_ENUM);
          break;
    }
}

void glLoadIdentity(void) {
    GLContext *context = gl_GetCurrentContext();
    MatrixStack_LoadIdentity(context->matrix[context->matrix_mode]);
}

void glLoadMatrixx (const GLfixed *m) {
    GLContext *context = gl_GetCurrentContext();
    mat4_from_array(MatrixStack_GetCurrent(context->matrix[context->matrix_mode]), m);
}

void glMultMatrixx(const GLfixed * m) {
    GLContext *context = gl_GetCurrentContext();
    mat4 temp;
    mat4_from_array(&temp, m);
    MatrixStack_MultMatrix(context->matrix[context->matrix_mode], &temp);
}

void glPushMatrix(void) {
    GLContext *context = gl_GetCurrentContext();
    if (!MatrixStack_Push(context->matrix[context->matrix_mode])) gl_SetError(context, GL_STACK_OVERFLOW);
}

void glPopMatrix(void) {
    GLContext *context = gl_GetCurrentContext();
    if (!MatrixStack_Pop(context->matrix[context->matrix_mode])) gl_SetError(context, GL_STACK_UNDERFLOW);
}

void glTranslatex (GLfixed x, GLfixed y, GLfixed z) {
#define M_SUM(row) fixed_mul( M4(m->M, row, 0), x) + fixed_mul( M4(m->M, row, 1), y) + fixed_mul( M4(m->M, row, 2), z) + M4(m->M, row, 3)
    GLContext *context = gl_GetCurrentContext();
    mat4 *m = MatrixStack_GetCurrent(context->matrix[context->matrix_mode]);
    if (context->matrix[context->matrix_mode]->identity) {
        M4(m->M, 0, 3) = x;
        M4(m->M, 1, 3) = y;
        M4(m->M, 2, 3) = z;
        context->matrix[context->matrix_mode]->identity = GL_FALSE;
    } else {
        M4(m->M, 0, 3) = M_SUM(0);
        M4(m->M, 1, 3) = M_SUM(1);
        M4(m->M, 2, 3) = M_SUM(2);
        M4(m->M, 3, 3) = M_SUM(3);
    }
    context->matrix[context->matrix_mode]->modified = GL_TRUE;
#undef M_SUM
}

void glScalex (GLfixed x, GLfixed y, GLfixed z) {
    GLContext *context = gl_GetCurrentContext();
    mat4 *m = MatrixStack_GetCurrent(context->matrix[context->matrix_mode]);
    if (context->matrix[context->matrix_mode]->identity) {
        M4(m->M, 0, 0) = x;
        M4(m->M, 1, 1) = y;
        M4(m->M, 2, 2) = z;
        context->matrix[context->matrix_mode]->identity = GL_FALSE;
    } else {
        if (x != fixed_int(1)) {
            M4(m->M, 0, 0) = fixed_mul( M4(m->M, 0, 0), x);
            M4(m->M, 1, 0) = fixed_mul( M4(m->M, 1, 0), x);
            M4(m->M, 2, 0) = fixed_mul( M4(m->M, 2, 0), x);
            M4(m->M, 3, 0) = fixed_mul( M4(m->M, 3, 0), x);
        }
        if (y != fixed_int(1)) {
            M4(m->M, 0, 1) = fixed_mul( M4(m->M, 0, 1), y);
            M4(m->M, 1, 1) = fixed_mul( M4(m->M, 1, 1), y);
            M4(m->M, 2, 1) = fixed_mul( M4(m->M, 2, 1), y);
            M4(m->M, 3, 1) = fixed_mul( M4(m->M, 3, 1), y);
        }
        if (z != fixed_int(1)) {
            M4(m->M, 0, 2) = fixed_mul( M4(m->M, 0, 2), z);
            M4(m->M, 1, 2) = fixed_mul( M4(m->M, 1, 2), z);
            M4(m->M, 2, 2) = fixed_mul( M4(m->M, 2, 2), z);
            M4(m->M, 3, 2) = fixed_mul( M4(m->M, 3, 2), z);
        }
    }
    context->matrix[context->matrix_mode]->modified = GL_TRUE;
}

void glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z) {
    GLContext *context = gl_GetCurrentContext();
    mat4 m;
    GLfixed w;
    GLfixed c, s, one_c;
    GLfixed xx, yy, zz;
    GLfixed xy, xz, yz;
    GLfixed xs, ys, zs;
    GLint flags;

    flags = (x != 0) | ((y != 0)<<1) | ((z != 0) << 2);
    if (flags == 0) return;

    mat4_identity(&m);

    switch (flags) {
        case 1 : // x
          if (x < 0) angle = -angle;
            c = fixed_cos(angle);
            s = fixed_sin(angle);

            M4(m.M, 1, 1) = c;
            M4(m.M, 2, 1) = s;

            M4(m.M, 1, 2) = -s;
            M4(m.M, 2, 2) = c;
            break;
        case 2 : // y
            if (y < 0) angle = -angle;

            c = fixed_cos(angle);
            s = fixed_sin(angle);

            M4(m.M, 0, 0) = c;
            M4(m.M, 2, 0) = -s;

            M4(m.M, 0, 2) = s;
            M4(m.M, 2, 2) = c;
          break;
        case 4 : // z
            if (z < 0) angle = -angle;

            c = fixed_cos(angle);
            s = fixed_sin(angle);

            M4(m.M, 0, 0) = c;
            M4(m.M, 1, 0) = s;

            M4(m.M, 0, 1) = -s;
            M4(m.M, 1, 1) = c;
          break;
        default :
            w = fixed_mul(x, x) + fixed_mul(y, y) + fixed_mul(z, z);

            if (w != fixed_int(1)) {
                w = fixed_sqrt(w);
                if (w == 0) return;

                w = fixed_div(fixed_int(1), w);
                x = fixed_mul(x, w);
                y = fixed_mul(y, w);
                z = fixed_mul(z, w);
            }

            c = fixed_cos(angle);
            s = fixed_sin(angle);

            one_c = fixed_int(1) - c;
            xx = fixed_mul(x, x);
            yy = fixed_mul(y, y);
            zz = fixed_mul(z, z);
            xy = fixed_mul(x, y);
            xz = fixed_mul(x, z);
            yz = fixed_mul(y, z);
            xs = fixed_mul(x, s);
            ys = fixed_mul(y, s);
            zs = fixed_mul(z, s);

            M4(m.M, 0, 0) = fixed_mul(xx, one_c) + c;
            M4(m.M, 1, 0) = fixed_mul(xy, one_c) + zs;
            M4(m.M, 2, 0) = fixed_mul(xz, one_c) - ys;

            M4(m.M, 0, 1) = fixed_mul(xy, one_c) - zs;
            M4(m.M, 1, 1) = fixed_mul(yy, one_c) + c;
            M4(m.M, 2, 1) = fixed_mul(yz, one_c) + xs;

            M4(m.M, 0, 2) = fixed_mul(xz, one_c) + ys;
            M4(m.M, 1, 2) = fixed_mul(yz, one_c) - xs;
            M4(m.M, 2, 2) = fixed_mul(zz, one_c) + c;
            break;
    }

    MatrixStack_MultMatrix(context->matrix[context->matrix_mode], &m);
}

void glOrthox (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar) {
    GLContext *context = gl_GetCurrentContext();
    mat4 m;
    GLfixed right_left;
    GLfixed top_bottom;
    GLfixed far_near;
    //int i, j;

    if (left == right || top == bottom || zNear == zFar) {
        gl_SetError(gl_GetCurrentContext(), GL_INVALID_VALUE);
    }

    mat4_identity(&m);

    right_left = fixed_div(fixed_int(1), right - left);
    top_bottom = fixed_div(fixed_int(1), top - bottom);
    far_near = fixed_div(fixed_int(1), zFar - zNear);

    M4(m.M, 0, 0) = (right_left << 1);
    M4(m.M, 1, 1) = (top_bottom << 1);
    M4(m.M, 2, 2) = - (far_near << 1);
    M4(m.M, 3, 3) = fixed_int(1);

    M4(m.M, 0, 3) = - fixed_mul(right + left, right_left);
    M4(m.M, 1, 3) = - fixed_mul(top + bottom, top_bottom);
    M4(m.M, 2, 3) = - fixed_mul(zFar + zNear, far_near);

    MatrixStack_MultMatrix(context->matrix[context->matrix_mode], &m);
}

void glFrustumx (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar) {
    GLContext *context = gl_GetCurrentContext();
    mat4 m;
    GLfixed right_left;
    GLfixed top_bottom;
    GLfixed far_near;
    GLfixed zNear_2;

    if (left == right || top == bottom || zNear == zFar || zNear < 0 || zFar < 0) {
        gl_SetError(gl_GetCurrentContext(), GL_INVALID_VALUE);
    }

    mat4_identity(&m);

    right_left = fixed_div(fixed_int(1), right - left);
    top_bottom = fixed_div(fixed_int(1), top - bottom);
    far_near = fixed_div(fixed_int(1), zFar - zNear);
    zNear_2 = zNear << 1;

    M4(m.M, 0, 0) = fixed_mul(zNear_2, right_left);
    M4(m.M, 1, 1) = fixed_mul(zNear_2, top_bottom);

    M4(m.M, 0, 2) = fixed_mul(right + left, right_left);
    M4(m.M, 1, 2) = fixed_mul(top + bottom, top_bottom);
    M4(m.M, 2, 2) = - fixed_mul(zFar + zNear, far_near);

    M4(m.M, 3, 2) = - fixed_int(1);
    M4(m.M, 2, 3) = - fixed_mul(fixed_mul(zFar, zNear_2), far_near);
    M4(m.M, 3, 3) = fixed_int(0);

    MatrixStack_MultMatrix(context->matrix[context->matrix_mode], &m);
}
