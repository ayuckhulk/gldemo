#include "common.h"

void VertexList_Clear(VertexList *list) {
    int i;
    list->count = 0;
    list->last = 0;
    list->loop = 1;

    list->next[0] = 0;
    for (i = 1;i <= MAX_VERTEX;i++) list->next[i] = i+1;
    list->next[MAX_VERTEX + 1] = 1;
}

int VertexList_RemoveIndexAfter(VertexList *list, int index) {
    int i = list->next[index];
    list->next[index] = list->next[list->next[index]];
    return i;
}

void VertexList_InsertIndexAfter(VertexList *list, int new_index, int index) {
    list->next[new_index] = list->next[index];
    list->next[index] = new_index;
}

void VertexList_RemoveAfter(VertexList *list, int index) {
    if (list->next[index] == 0) index = 0;
    else if (list->last == list->next[index]) list->last = index;
    VertexList_InsertIndexAfter(list, VertexList_RemoveIndexAfter(list, index), 1);
    list->count--;
}

int VertexList_AllocAfter(VertexList *list, int index) {
    int i;

    if (list->next[1] == 1) return 0;

    i = VertexList_RemoveIndexAfter(list, 1);
    VertexList_InsertIndexAfter(list, i, index);

    if (list->last == index) list->last = i;
    list->count++;

    return i;
}

int VertexList_InsertAfter(VertexList *list, Vertex v, int index) {
    int i;

    i = VertexList_AllocAfter(list, index);
    if (i) list->vertex[i] = v;

    return i;
}

Vertex *VertexList_Append(VertexList *list, Vertex *v) {
    int i;

    i = VertexList_AllocAfter(list, list->last);
    if (i) {
        list->vertex[i] = *v;
        return list->vertex + i;
    } else return NULL;
}

int VertexList_First(VertexList *list) {
    return list->next[0];
}

int VertexList_Next(VertexList *list, int index) {
    int i = list->next[index];
    if (list->loop) {
        if (i) return i;
        else return list->next[0];
    } else return i;
}

void VertexList_Interpolate(VertexList *list, GLint i0, GLint i1, GLint i) {
    GLContext *context = gl_GetCurrentContext();
    Vertex *v0;
    Vertex *v1;
    Vertex *v;
    GLfixed t;
    GLint j;

//    GLfixed v0_1w, v1_1w, v0_sw, v1_sw, v0_tw, v1_tw;
//    GLfixed v_sw, v_tw, v_1w, v_s, v_t, v_w;

    if (-list->clip[i0] < list->clip[i1]) {
        GLint it;
        it = i0;
        i0 = i1;
        i1 = it;
    }
    v0 = list->vertex + i0;
    v1 = list->vertex + i1;
    t = fixed_div( list->clip[i0], list->clip[i0] - list->clip[i1]);

    v = list->vertex + i;

/*    v0_1w = divx22(itox22(1), x16to22(v0->v[3]));
    v1_1w = divx22(itox22(1), x16to22(v1->v[3]));
    v0_sw = (fixed_div(v0->t[0][0], v0->v[3])); // >> TEXCOORD22_SHIFT;
    v1_sw = (fixed_div(v1->t[0][0], v1->v[3])); // >> TEXCOORD22_SHIFT;
    v0_tw = (fixed_div(v0->t[0][1], v0->v[3])); // >> TEXCOORD22_SHIFT;
    v1_tw = (fixed_div(v1->t[0][1], v1->v[3])); // >> TEXCOORD22_SHIFT;
 
    v_sw = v0_sw + (fixed_mul(v1_sw - v0_sw, t) >> 6);
    v_tw = v0_tw + (fixed_mul(v1_tw - v0_tw, t) >> 6);
    v_1w = v0_1w + (fixed_mul(v1_1w - v0_1w, t) >> 6);

    v_w = divx22(itox22(1), v_1w) >> 6;
    v_s = fixed_mul(v_sw, v_w);
    v_t = fixed_mul(v_tw, v_w);
*/
    for (j = 0;j < 4;j++) 
        v->v.xyzw[j] = fixed_interpolate(v0->v.xyzw[j], v1->v.xyzw[j], t);

    if (context->shade_model == GL_SMOOTH)
    for (j = 0;j < 4;j++) v->color[j] = fixed_interpolate(v0->color[j], v1->color[j], t);

    for (j = 0;j < MAX_TEXTURE_UNITS;j++) {
        if (context->TU[j]->enabled) {
            v->t[j].p.x = fixed_interpolate(v0->t[j].p.x, v1->t[j].p.x, t);
            v->t[j].p.y = fixed_interpolate(v0->t[j].p.y, v1->t[j].p.y, t);
            //v->t[j][0] = v0->t[j][0] + fixed_muldiv(v1->t[j][0] - v0->t[j][0], list->clip[i0], list->clip[i0] - list->clip[i1]);
            //v->t[j][1] = v0->t[j][1] + fixed_muldiv(v1->t[j][1] - v0->t[j][1], list->clip[i0], list->clip[i0] - list->clip[i1]);
            //v->t[j][0] = v_s;
            //v->t[j][1] = v_t;

        }
    }

    list->clip[i] = 0;

/*    if (-list->clip[i0] < list->clip[i1]) {
        GLint it;
        it = i0;
        i0 = i1;
        i1 = it;
    }
    v0 = list->vertex + i0;
    v1 = list->vertex + i1;
    //t = fixed_div( list->clip[i0], list->clip[i0] - list->clip[i1]);

    v = list->vertex + i;

    v->v[0] = v0->v[0] + fixed_muldiv(v1->v[0] - v0->v[0], list->clip[i0], list->clip[i0] - list->clip[i1]);
    v->v[1] = v0->v[1] + fixed_muldiv(v1->v[1] - v0->v[1], list->clip[i0], list->clip[i0] - list->clip[i1]);
    v->v[2] = v0->v[2] + fixed_muldiv(v1->v[2] - v0->v[2], list->clip[i0], list->clip[i0] - list->clip[i1]);
    v->v[3] = v0->v[3] + fixed_muldiv(v1->v[3] - v0->v[3], list->clip[i0], list->clip[i0] - list->clip[i1]);

    for (j = 0;j < MAX_TEXTURE_UNITS;j++) {
        if (context->TU[j]->enabled) {
            v->t[j][0] = v0->t[j][0] + fixed_muldiv(v1->t[j][0] - v0->t[j][0], list->clip[i0], list->clip[i0] - list->clip[i1]);
            v->t[j][1] = v0->t[j][1] + fixed_muldiv(v1->t[j][1] - v0->t[j][1], list->clip[i0], list->clip[i0] - list->clip[i1]);
        }
    }

    list->clip[i] = 0;*/
}

GLboolean VertexList_ClipCoord(VertexList *list, GLint coord, GLint dir) {
    vec4 *v;
    int i, next, first;
    int remove_index;
    int remove_count;
    int clip_code;
    int new_vertex;

    clip_code = 1;
    i = list->next[0];

    while (i != 0) {
        v = &(list->vertex[i].v);

        list->clip[i] = list->vertex[i].v.p.w - dir * list->vertex[i].v.xyzw[coord];
        if (list->clip[i] > 0) clip_code = 0;

        i = list->next[i];
    }

    if (clip_code) return GL_TRUE;

    if (list->count == 1) return GL_FALSE; // for GL_POINTS

    remove_index = 0;
    remove_count = 0;

    first = VertexList_First(list);
    i = first;
    do {
        next = VertexList_Next(list, i);

        if (next) {
            clip_code = ((list->clip[next] < 0) << 1) | (list->clip[i] < 0);

            switch (clip_code) {
            case 1 :
                new_vertex = VertexList_AllocAfter(list, i);
                VertexList_Interpolate(list, i, next, new_vertex);
                break;
            case 2 :
                new_vertex = VertexList_AllocAfter(list, i);
                remove_index = new_vertex;
                VertexList_Interpolate(list, next, i, new_vertex);
                remove_count++;
                break;
            case 3 :
                remove_count++;
                break;
            default :
                break;
            }
        }
        i = next;
    } while (i != first && i != 0);

    while (remove_count--) VertexList_RemoveAfter(list, remove_index);

    return (list->count == 0);
}

GLboolean VertexList_Clip(VertexList *list) {
#define X 0
#define Y 1
#define Z 2
    if (list->count == 2) list->loop = 0; // for GL_LINES

    if (VertexList_ClipCoord(list, Z, 1)) return GL_TRUE;
    if (VertexList_ClipCoord(list, Z, -1)) return GL_TRUE;
    if (VertexList_ClipCoord(list, X, -1)) return GL_TRUE;
    if (VertexList_ClipCoord(list, X, 1)) return GL_TRUE;
    if (VertexList_ClipCoord(list, Y, 1)) return GL_TRUE;
    if (VertexList_ClipCoord(list, Y, -1)) return GL_TRUE;
    return GL_FALSE;
}
