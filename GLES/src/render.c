#include "common.h"

#define subi(v, v1, v2) { \
    v[0] = fixed_toint_round(v1.p.x - v2.p.x); \
    v[1] = fixed_toint_round(v1.p.y - v2.p.y); \
}

#define subiw(v, v1, v2) { \
    v[0] = fixed_toint_round(fixed_div(v1[0], v1[3]) - fixed_div(v2[0], v2[3])); \
    v[1] = fixed_toint_round(fixed_div(v1[1], v1[3]) - fixed_div(v2[1], v2[3])); \
}

#define crossz(v1, v2) ((v1[0] * v2[1]) - (v1[1] * v2[0]))

GLboolean gl_BackFace(GLContext *context, Vertex *v0, Vertex *v1, Vertex *v2) {
    GLint a[2];
    GLint b[2];
    GLint z;

    subi(a, v1->v, v0->v);
    subi(b, v2->v, v1->v);

    z = crossz(a, b);
return 1;
    switch (context->front_face) {
    case GL_CW: return (z < 0);
    default: return (z > 0);
    }
}

GLboolean gl_CullTest(GLContext *context, Vertex *v0, Vertex *v1, Vertex *v2) {
// if cull test is true, then draw triangle
    GLint a[2];
    GLint b[2];
    GLint z;

    subi(a, v1->v, v0->v);
    subi(b, v2->v, v1->v);

    z = crossz(a, b);

    switch (context->cull_face_mode) {
    case GL_BACK:
        return (context->front_face == GL_CCW) ^ (z < 0);
    case GL_FRONT:
        return (context->front_face == GL_CW) ^ (z < 0);
    case GL_FRONT_AND_BACK:
        return GL_FALSE;
    }

    return GL_FALSE;
}

#undef sub
#undef crossz

void RenderingBuffer_Begin(RenderingBuffer *buffer, GLenum mode) {
    GLContext *context = gl_GetCurrentContext();

    buffer->mode = mode;
    buffer->n = 0;
    buffer->index = 0;

    buffer->data_locked = (context->shade_model == GL_FLAT);

    VertexList_Clear(&buffer->list);

    /*
     GLfixed *projection_matrix;
     GLfixed *modelview_matrix;
     GLint i, i0, i1;

     buffer->modelview = MatrixStack_GetCurrent(context->matrix[MODELVIEW_MATRIX]);
     buffer->projection = MatrixStack_GetCurrent(context->matrix[PROJECTION_MATRIX]);

     gl_MultMatrixx(buffer->clip_matrix, projection_matrix, modelview_matrix);

     #define M(row, col) buffer->clip_matrix[row + col * 4]
     // [x, y, z]
     for (i = 0;i < 3;i++) {
     i0 = i << 1;
     i1 = i0 + 1;

     // -w < [x, y, z]
     buffer->clip_plane[i0][0] = M(3, 0) + M(i, 0);
     buffer->clip_plane[i0][1] = M(3, 1) + M(i, 1);
     buffer->clip_plane[i0][2] = M(3, 2) + M(i, 2);
     buffer->clip_plane[i0][3] = M(3, 3) + M(i, 3);

     //  w > [x, y, z]
     buffer->clip_plane[i1][0] = M(3, 0) - M(i, 0);
     buffer->clip_plane[i1][1] = M(3, 1) - M(i, 1);
     buffer->clip_plane[i1][2] = M(3, 2) - M(i, 2);
     buffer->clip_plane[i1][3] = M(3, 3) - M(i, 3);
     }
     #undef M
     */
}

void RenderingBuffer_End(RenderingBuffer *buffer) {
}


inline Vertex *RenderingBuffer_CurrentVertex(RenderingBuffer *buffer) {
    return buffer->vertex + buffer->index;
}

inline VertexData *RenderingBuffer_CurrentVertexData(RenderingBuffer *buffer) {
    return buffer->data + buffer->index;
}

void RenderingBuffer_ProcessVertex(RenderingBuffer *buffer) {
    GLContext *context = gl_GetCurrentContext();
    GLboolean flat = context->shade_model == GL_FLAT;
    GLboolean render = GL_FALSE;

    Vertex *v0;
    Vertex *v1;
    Vertex *v2;


    buffer->n++;

    switch (buffer->mode) {
    case GL_POINTS:
    case GL_LINES:
        if (buffer->index == 1) {
            render = GL_TRUE;
            buffer->v0 = 0;
            buffer->v1 = 1;
            buffer->index = 0;
        } else
            buffer->index++;

        buffer->data_locked = flat && (buffer->index != 1);

        break;
    case GL_LINE_STRIP:
        if (buffer->index == 1) {
            render = GL_TRUE;
            buffer->v0 = 0;
            buffer->v1 = 1;
            buffer->index = 1;
        } else
            buffer->index++;

        buffer->data_locked = flat && (buffer->index != 1);

        break;
    case GL_LINE_LOOP:
        break;
    case GL_TRIANGLES:
        if (buffer->index == 2) {
            render = GL_TRUE;
            buffer->v0 = 0;
            buffer->v1 = 1;
            buffer->v2 = 2;
            buffer->index = 0;
        } else
            buffer->index++;

        buffer->data_locked = flat && (buffer->index != 2);

        break;
    case GL_TRIANGLE_FAN:
        if (buffer->index == 2) {
            render = GL_TRUE;
            buffer->v0 = 0;
            buffer->v1 = 1;
            buffer->v2 = 2;
            buffer->index = 1;
        } else if (buffer->n > 2 && buffer->index == 1) {
            render = GL_TRUE;
            buffer->v0 = 0;
            buffer->v1 = 2;
            buffer->v2 = 1;
            buffer->index = 2;
        } else
            buffer->index++;

        buffer->data_locked = flat && (buffer->n <= 1);

        break;
    case GL_TRIANGLE_STRIP:
        if (buffer->n > 2) {
            render = GL_TRUE;
            if (buffer->n & 0x1) { // odd
                buffer->v0 = (buffer->index - 2) & 0x3;
                buffer->v1 = (buffer->index - 1) & 0x3;
                buffer->v2 = buffer->index;
            } else { // even
                buffer->v0 = (buffer->index - 1) & 0x3;
                buffer->v1 = (buffer->index - 2) & 0x3;
                buffer->v2 = buffer->index;
            }
        }
        buffer->index = (buffer->index + 1) & 0x3;

        buffer->data_locked = flat && (buffer->n <= 1);

        break;
    }

    if (render) {
        switch(buffer->mode) {
        case GL_POINTS:
            v0 = VertexList_Append(&buffer->list, buffer->vertex + buffer->v0);

            if (flat) memcpy(context->flat_color, v0->color, sizeof(GLubyte) * 4);

            //RenderingBuffer_RenderPoint(buffer);
            break;
        case GL_LINES:
        case GL_LINE_STRIP:
            v0 = VertexList_Append(&buffer->list, buffer->vertex + buffer->v0);
            v1 = VertexList_Append(&buffer->list, buffer->vertex + buffer->v1);

            if (flat) memcpy(context->flat_color, v1->color, sizeof(GLubyte) * 4);

            //RenderingBuffer_RenderLine(buffer);
            break;
        case GL_TRIANGLES:
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
            v0 = VertexList_Append(&buffer->list, buffer->vertex + buffer->v0);
            v1 = VertexList_Append(&buffer->list, buffer->vertex + buffer->v1);
            v2 = VertexList_Append(&buffer->list, buffer->vertex + buffer->v2);

            if (context->lighting_enabled) {
                GLboolean back;

                if (context->tbs) {
                    back = VertexList_BackFace(&buffer->list);
                    VertexList_Clear(&buffer->list);
                    v0 = VertexList_Append(&buffer->list, buffer->vertex + buffer->v0);
                    v1 = VertexList_Append(&buffer->list, buffer->vertex + buffer->v1);
                    v2 = VertexList_Append(&buffer->list, buffer->vertex + buffer->v2);
                } else back = GL_FALSE;

                if (flat) {
                    Ligths_ComputeColor(context->lights, v2, buffer->data + buffer->v2, back);
                } else {
                    Ligths_ComputeColor(context->lights, v0, buffer->data + buffer->v0, back);
                    Ligths_ComputeColor(context->lights, v1, buffer->data + buffer->v1, back);
                    Ligths_ComputeColor(context->lights, v2, buffer->data + buffer->v2, back);
                }
            }

        	context->grey_color = GL_FALSE;
            if (flat) {
            	memcpy(context->flat_color, v2->color, sizeof(GLubyte) * 4);
            	if ((v2->color[0] == v2->color[1]) && (v2->color[1] == v2->color[2]))
            		context->grey_color = GL_TRUE;
            } else {
            	if ((v0->color[0] == v0->color[1]) && (v0->color[1] == v0->color[2]))
            		if ((v1->color[0] == v1->color[1]) && (v1->color[1] == v1->color[2]))
            			if ((v2->color[0] == v2->color[1]) && (v2->color[1] == v2->color[2]))
            				context->grey_color = GL_TRUE;
            }

            RenderingBuffer_RenderTriangle(buffer);

            break;
        }
        VertexList_Clear(&buffer->list);
    }
}
/*
 void gl_ProjectVertex(GLContext *context, Vertex *v) {
 GLfixed *projection = MatrixStack_GetCurrent(context->matrix[PROJECTION_MATRIX]);
 GLfixed result[4];
 //int i;

 gl_MultVectorx(result, projection, v->v);

 v->v[0] = result[0];
 v->v[1] = result[1];
 v->v[2] = result[2];
 v->v[3] = result[3];

 //for (i = 0; i < 4;i++) printf("%.4f ", fixed_tofloat(v->v[i])); printf("\n");
 }*/

void gl_VertexToViewport(GLContext *context, Vertex *v) {
    GLfixed xc, yc, zc, wc;
    //GLfixed one_over_wc;

    xc = v->v.p.x;
    yc = v->v.p.y;
    zc = v->v.p.z;
    wc = v->v.p.w;

    //one_over_wc = fixed_div(fixed_int(1), wc);

    // xw
    v->v.p.x
            = fixed_round(fixed_int(context->view_x) +  fixed_muldiv((context->view_width - 0) >> 1, xc, wc) + (fixed_int(context->view_width - 0) >> 1));
    // yw
    v->v.p.y
            = fixed_round(fixed_int(context->height - context->view_y - 1) -  fixed_muldiv( (context->view_height) >> 1, yc, wc ) - (fixed_int(context->view_height) >> 1));
    // zw
    zc = fixed_mul(context->zfar, zc + wc) - fixed_mul(context->znear, zc - wc);
    v->v.p.z =  fixed_muldiv(ZBUFFER_SCALE, zc, wc) >> 1;

    //v->v.p.z = fixed_muldiv(ZBUFFER_SCALE, zc + wc, wc) >> 1;
    if (v->v.p.z < 0)
        v->v.p.z = 0;

    //for (i = 0; i < 4;i++) printf("%.4f ", fixed_tofloat(v->v[i])); printf("\n-----\n");
}

drawtriangle_func gl_GetDrawTriangleFunc(GLContext *context) {
    GLint index = 0;

    if (context->TU[0]->enabled && Texture_IsComplete (context->TU[0]->texture)) {
        switch (context->TU[0]->env_mode) {
        case GL_REPLACE:
        	index |= 0x10;
        	break;
        case GL_MODULATE:
        	index |= 0x20;
        	if (context->grey_color) {
        		index |= 0x10;
        	}
        	break;
        }
        if (context->TU[0]->texture->min_filter == GL_NEAREST_MIPMAP_NEAREST)
        	index |= 0x40;

        if (context->perspective_correction)
            index |= 0x4;
    }


    if (context->blend_enabled)
        index |= 0x8;

    if (context->depth_test_enabled)
        index |= 0x2;
    if (context->alpha_test_enabled)
    	index |= 0x1;

    return drawtriangle[index];
}

GLboolean VertexList_BackFace(VertexList *list) {
    GLContext *context = gl_GetCurrentContext();
    GLint first, i, i1, next;
    GLint S;

    if (VertexList_Clip(list)) return GL_FALSE;

    first = VertexList_First(list);

    i = list->next[first];
    next = list->next[i];

    gl_VertexToViewport(context, list->vertex + first);
    gl_VertexToViewport(context, list->vertex + i);
    gl_VertexToViewport(context, list->vertex + next);

    while (1) {
        i = next;
        next = list->next[next];

        if (next)
            gl_VertexToViewport(context, list->vertex + next);
        else
            break;
    }

    i = first;
    i1 = list->next[i];

    S = 0;
    for (next = 0;next < list->count;next++) {
        S += fixed_toint_round(list->vertex[i].v.p.x) * fixed_toint_round(list->vertex[i1].v.p.y);
        S -= fixed_toint_round(list->vertex[i1].v.p.x) * fixed_toint_round(list->vertex[i].v.p.y);
        i = i1;
        i1 = list->next[i1];
        if (i1 == 0) i1 = first;
    }

    if (context->front_face == GL_CCW) return (S <= 0);
    else return (S > 0);
}

void RenderingBuffer_RenderTriangle(RenderingBuffer *buffer) {
    GLContext *context = gl_GetCurrentContext();
    VertexList *list = &buffer->list;
    GLint first, i, next;
    drawtriangle_func gl_DrawTriangle;

    if (VertexList_Clip(list)) return;

    gl_DrawTriangle = gl_GetDrawTriangleFunc(context);

    first = VertexList_First(list);
    i = list->next[first];
    next = list->next[i];

    gl_VertexToViewport(context, list->vertex + first);
    gl_VertexToViewport(context, list->vertex + i);
    gl_VertexToViewport(context, list->vertex + next);

    while (1) {
        if (!context->cull_face_enabled || !gl_CullTest(context, list->vertex
                            + first, list->vertex + i, list->vertex + next))
            gl_DrawTriangle(context, list->vertex + first, list->vertex + i, list->vertex + next);

        i = next;
        next = list->next[next];

        if (next)
            gl_VertexToViewport(context, list->vertex + next);
        else
            break;
    }
}
