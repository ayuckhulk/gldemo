#include "common.h"

void rgba_set(GLfixed *rgba, GLfixed r, GLfixed g, GLfixed b, GLfixed a) {
    rgba[0] = r;
    rgba[1] = g;
    rgba[2] = b;
    rgba[3] = a;
}

void rgb_add(GLfixed *result, GLfixed *c1, GLfixed *c2) {
    result[0] = c1[0] + c2[0];
    result[1] = c1[1] + c2[1];
    result[2] = c1[2] + c2[2];
}

void rgb_mul(GLfixed *result, GLfixed *c1, GLfixed *c2) {
    result[0] = fixed_mul(c1[0], c2[0]);
    result[1] = fixed_mul(c1[1], c2[1]);
    result[2] = fixed_mul(c1[2], c2[2]);
}

void rgb_scale(GLfixed *result, GLfixed a, GLfixed *rgb) {
    result[0] = fixed_mul(a, rgb[0]);
    result[1] = fixed_mul(a, rgb[1]);
    result[2] = fixed_mul(a, rgb[2]);
}

void rgba_from_array(GLfixed *rgba, GLubyte *array) {
    rgba[0] = fixed_int(array[0]) / 255;
    rgba[1] = fixed_int(array[1]) / 255;
    rgba[2] = fixed_int(array[2]) / 255;
    rgba[3] = fixed_int(array[3]) / 255;
}

void rgba_to_array(GLfixed *rgba, GLubyte *array) {
    GLint i = fixed_toint(255 * rgba[0]);
    if (i > 255) i = 255; else if (i < 0) i = 0;
    array[0] = i;
    i = fixed_toint(255 * rgba[1]);
    if (i > 255) i = 255; else if (i < 0) i = 0;
    array[1] = i;
    i = fixed_toint(255 * rgba[2]);
    if (i > 255) i = 255; else if (i < 0) i = 0;
    array[2] = i;
    i = fixed_toint(255 * rgba[3]);
    if (i > 255) i = 255; else if (i < 0) i = 0;
    array[3] = i;
}

GLfixed light_vec3_dot(vec3 *v1, vec3 *v2) {
    GLfixed result = vec3_dot(v1, v2);
    return (result < 0 ? 0 : result);
}

void vec3_direction(vec3 *v, vec4 *P1, vec4 *P2) {
    if (P1->p.w == fixed_int(0)) {
        if (P2->p.w == fixed_int(0)) {
            v->xyz[0] = P2->xyzw[0] - P1->xyzw[0];
            v->xyz[1] = P2->xyzw[1] - P1->xyzw[1];
            v->xyz[2] = P2->xyzw[2] - P1->xyzw[2];
        } else {
            v->xyz[0] = - P1->xyzw[0];
            v->xyz[1] = - P1->xyzw[1];
            v->xyz[2] = - P1->xyzw[2];
        }
    } else {
        if (P2->p.w == fixed_int(0)) {
            v->xyz[0] = P2->xyzw[0];
            v->xyz[1] = P2->xyzw[1];
            v->xyz[2] = P2->xyzw[2];
        } else {
            if (P2->p.w != fixed_int(1)) {
                v->xyz[0] = fixed_div(P2->xyzw[0], P2->xyzw[3]);
                v->xyz[1] = fixed_div(P2->xyzw[1], P2->xyzw[3]);
                v->xyz[2] = fixed_div(P2->xyzw[2], P2->xyzw[3]);
            } else {
                v->xyz[0] = P2->xyzw[0];
                v->xyz[1] = P2->xyzw[1];
                v->xyz[2] = P2->xyzw[2];
            }
            if (P1->p.w != fixed_int(1)) {
                v->xyz[0] -= fixed_div(P1->xyzw[0], P1->xyzw[3]);
                v->xyz[1] -= fixed_div(P1->xyzw[1], P1->xyzw[3]);
                v->xyz[2] -= fixed_div(P1->xyzw[2], P1->xyzw[3]);
            } else {
                v->xyz[0] -= P1->xyzw[0];
                v->xyz[1] -= P1->xyzw[1];
                v->xyz[2] -= P1->xyzw[2];
            }
        }
    }
}

void Lights_Init(Light *lights) {
    GLuint i;

    for (i = 0; i < MAX_LIGHTS; i++) {
        lights[i].enabled = GL_FALSE;
        rgba_set( lights[i].ac, fixed_float(0.0f), fixed_float(0.0f), fixed_float(0.0f), fixed_float(1.0f));
        if (i > 0) {
            rgba_set( lights[i].dc, fixed_float(0.0f), fixed_float(0.0f), fixed_float(0.0f), fixed_float(1.0f));
            rgba_set( lights[i].sc, fixed_float(0.0f), fixed_float(0.0f), fixed_float(0.0f), fixed_float(1.0f));
        } else {
            rgba_set( lights[i].dc, fixed_float(1.0f), fixed_float(1.0f), fixed_float(1.0f), fixed_float(1.0f));
            rgba_set( lights[i].sc, fixed_float(1.0f), fixed_float(1.0f), fixed_float(1.0f), fixed_float(1.0f));
        }
        vec4_set( &lights[i].P, fixed_float(0.0f), fixed_float(0.0f), fixed_float(1.0f), fixed_float(0.0f));
        vec3_set( &lights[i].sd, fixed_float(0.0f), fixed_float(0.0f), fixed_float(-1.0f));
        lights[i].sr = fixed_float(0.0f);
        lights[i].cr = fixed_float(180.0f);
        lights[i].k0 = fixed_float(1.0f);
        lights[i].k1 = fixed_float(0.0f);
        lights[i].k2 = fixed_float(0.0f);
    }
}

void Ligths_ComputeColor(Light *lights, Vertex *vertex, VertexData *data, GLboolean back) {
    GLContext *context = gl_GetCurrentContext();
    vec3 VP;
    vec3 h;
    GLfixed nVP;
    vec3 n;
    GLfixed len2;
    GLfixed len;

    GLfixed att;
    GLfixed spot;
    GLfixed f;

    GLfixed c[4];
    GLfixed a[3];
    GLfixed d[3];
    GLfixed s[3];

    GLuint i;

    if (context->color_material_enabled) {
        rgba_from_array( context->acm, vertex->color );
        rgba_from_array( context->dcm, vertex->color );
    }

    if (back) vec3_inverse(&n, &(data->n));
    else n = data->n;

    memcpy(c, context->ecm, sizeof(GLfixed) * 3);
    c[3] = context->dcm[3];

    rgb_mul(a, context->acm, context->acs);
    rgb_add(c, c, a);

    for (i = 0;i < MAX_LIGHTS;i++) {
        if (lights[i].enabled) {
            vec3_direction(&VP, &data->v, &lights[i].P);
            
            len2 = vec3_sqr_length(&VP);
            len = fixed_sqrt(len2);
            
            if (len != 0) {
                VP.xyz[0] = fixed_div(VP.xyz[0], len);
                VP.xyz[1] = fixed_div(VP.xyz[1], len);
                VP.xyz[2] = fixed_div(VP.xyz[2], len);
            }
            
            if (lights[i].P.p.w != fixed_int(0)) {
                att = lights[i].k0 + fixed_mul(lights[i].k1, len) + fixed_mul(lights[i].k2, len2);
                if (att == fixed_int(0)) att = fixed_int(1);
            } else {
                att = fixed_int(1);
            }

            if (lights[i].cr == fixed_int(180)) {
                spot = fixed_int(1);
            } else {
                GLfixed PVsd;
                vec3 sd;
                vec3 PV;
                vec3_inverse(&PV, &VP);
                vec3_normalize(&sd, &lights[i].sd);
                PVsd = light_vec3_dot(&PV, &sd);
                if (PVsd < fixed_cos(lights[i].cr)) {
                    spot = fixed_int(0);
                } else {
                    spot = fixed_pow(PVsd, lights[i].sr);
                }
            }

            nVP = light_vec3_dot(&n, &VP);

            if (nVP != fixed_int(0)) f = fixed_int(1);
            else f = fixed_int(0);

            h.xyz[0] = VP.xyz[0];
            h.xyz[1] = VP.xyz[1];
            h.xyz[2] = VP.xyz[2] + fixed_int(1);
            vec3_normalize(&h, &h);

            rgb_mul(a, context->acm, lights[i].ac);

            rgb_mul(d, context->dcm, lights[i].dc);
            rgb_scale(d, nVP, d);

            rgb_mul(s, context->scm, lights[i].sc);
            rgb_scale(s, fixed_mul(f, fixed_pow(light_vec3_dot(&n, &h), context->srm)), s);

            rgb_add(a, a, d);
            rgb_add(a, a, s);
            //vec3_scale(&a, fixed_div(spot, att), &a);

            rgb_add(c, c, a);
        }
    }
    
    rgba_to_array(c, vertex->color);
}

void glLightModelx (GLenum pname, GLfixed param) {
    GLContext *context = gl_GetCurrentContext();
    switch(pname) {
    case GL_LIGHT_MODEL_TWO_SIDE:
        if (param == fixed_int(0)) context->tbs = GL_FALSE;
        else context->tbs = GL_TRUE;
        break;
    default:
        gl_SetError(context, GL_INVALID_ENUM);
        break;
    }
}

void glLightModelxv (GLenum pname, const GLfixed *params) {
    GLContext *context = gl_GetCurrentContext();
    switch(pname) {
    case GL_LIGHT_MODEL_AMBIENT:
        memcpy(context->acs, params, 4 * sizeof(GLfixed));
        break;
    default:
        gl_SetError(context, GL_INVALID_ENUM);
        break;
    }
}

void glLightx (GLenum light, GLenum pname, GLfixed param) {
    GLContext *context = gl_GetCurrentContext();
    GLint i;
    i = light - GL_LIGHT0;
    if (i < 0 || i >= MAX_LIGHTS) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }
    switch(pname) {
    case GL_SPOT_EXPONENT:
        if (param < fixed_int(0) || param > fixed_int(128)) {
            gl_SetError(context, GL_INVALID_VALUE);
            return;
        }
        context->lights[i].sr = param;
        break;
    case GL_SPOT_CUTOFF:
        if ((param < fixed_int(0) || param > fixed_int(90)) && param != fixed_int(180)) {
            gl_SetError(context, GL_INVALID_VALUE);
            return;
        }
        context->lights[i].cr = param;
        break;
    case GL_CONSTANT_ATTENUATION:
        if (param < fixed_int(0)) {
            gl_SetError(context, GL_INVALID_VALUE);
            return;
        }
        context->lights[i].k0 = param;
        break;
    case GL_LINEAR_ATTENUATION:
        if (param < fixed_int(0)) {
            gl_SetError(context, GL_INVALID_VALUE);
            return;
        }
        context->lights[i].k1 = param;
        break;
    case GL_QUADRATIC_ATTENUATION:
        if (param < fixed_int(0)) {
            gl_SetError(context, GL_INVALID_VALUE);
            return;
        }
        context->lights[i].k2 = param;
        break;
    default:
        gl_SetError(context, GL_INVALID_ENUM);
        break;
    }
}

void glLightxv (GLenum light, GLenum pname, const GLfixed *params) {
    GLContext *context = gl_GetCurrentContext();
    GLint i;
    
    vec4 p;
    vec3 d;
    mat3 m;
        
    i = light - GL_LIGHT0;
    
    if (i < 0 || i >= MAX_LIGHTS) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }
    
    switch(pname) {
    case GL_AMBIENT:
        memcpy(context->lights[i].ac, params, 4 * sizeof(GLfixed));
        break;
    case GL_DIFFUSE:
        memcpy(context->lights[i].dc, params, 4 * sizeof(GLfixed));
        break;
    case GL_SPECULAR:
        memcpy(context->lights[i].sc, params, 4 * sizeof(GLfixed));
        break;
    case GL_POSITION:
        memcpy(p.xyzw, params, 4 * sizeof(GLfixed));
        mat4_mul_vec4(&context->lights[i].P, MatrixStack_GetCurrent(context->matrix[MODELVIEW_MATRIX]), &p);
        break;
    case GL_SPOT_DIRECTION:
        memcpy(d.xyz, params, 3 * sizeof(GLfixed));
        mat3_from_mat4(&m, MatrixStack_GetCurrent(context->matrix[MODELVIEW_MATRIX]));
        mat3_mul_vec3(&context->lights[i].sd, &m, &d);
        break;
    default:
        gl_SetError(context, GL_INVALID_ENUM);
        break;
    }
}

void glMaterialx (GLenum face, GLenum pname, GLfixed param) {
    GLContext *context = gl_GetCurrentContext();

    if (face != GL_FRONT_AND_BACK) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    switch(pname) {
    case GL_SHININESS:
        if (param < fixed_int(0) || param > fixed_int(128)) {
            gl_SetError(context, GL_INVALID_VALUE);
            return;
        }
        context->srm = param;
        break;
    default:
        gl_SetError(context, GL_INVALID_ENUM);
        break;
    }
}

void glMaterialxv (GLenum face, GLenum pname, const GLfixed *params) {
    GLContext *context = gl_GetCurrentContext();

    if (face != GL_FRONT_AND_BACK) {
        gl_SetError(context, GL_INVALID_ENUM);
        return;
    }

    switch(pname) {
    case GL_AMBIENT:
        memcpy(context->acm, params, 4 * sizeof(GLfixed));
        break;
    case GL_DIFFUSE:
        memcpy(context->dcm, params, 4 * sizeof(GLfixed));
        break;
    case GL_AMBIENT_AND_DIFFUSE:
        memcpy(context->acm, params, 4 * sizeof(GLfixed));
        memcpy(context->dcm, params, 4 * sizeof(GLfixed));
        break;
    case GL_SPECULAR:
        memcpy(context->scm, params, 4 * sizeof(GLfixed));
        break;
    case GL_EMISSION:
        memcpy(context->ecm, params, 4 * sizeof(GLfixed));
        break;
    default:
        gl_SetError(context, GL_INVALID_ENUM);
        break;
    }
}
