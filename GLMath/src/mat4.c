#include "glmath.h"
#include <string.h>

void mat4_copy(mat4 *dst, const mat4 *src) {
    memcpy(dst, src, sizeof(mat4));
}

void mat4_from_array(mat4 *result, const fixed *array) {
    memcpy(result->M, array, sizeof(fixed) * 16);
}

void mat4_mul(mat4 *result, const mat4 *m1, const mat4 *m2) {

#define M4_SUM(row, col) fixed_mul( M4(m1->M, row, 0), M4(m2->M, 0, col)) + \
                         fixed_mul( M4(m1->M, row, 1), M4(m2->M, 1, col)) + \
                         fixed_mul( M4(m1->M, row, 2), M4(m2->M, 2, col)) + \
                         fixed_mul( M4(m1->M, row, 3), M4(m2->M, 3, col))

    M4(result->M, 0, 0) = M4_SUM(0, 0);
    M4(result->M, 0, 1) = M4_SUM(0, 1);
    M4(result->M, 0, 2) = M4_SUM(0, 2);
    M4(result->M, 0, 3) = M4_SUM(0, 3);
    M4(result->M, 1, 0) = M4_SUM(1, 0);
    M4(result->M, 1, 1) = M4_SUM(1, 1);
    M4(result->M, 1, 2) = M4_SUM(1, 2);
    M4(result->M, 1, 3) = M4_SUM(1, 3);
    M4(result->M, 2, 0) = M4_SUM(2, 0);
    M4(result->M, 2, 1) = M4_SUM(2, 1);
    M4(result->M, 2, 2) = M4_SUM(2, 2);
    M4(result->M, 2, 3) = M4_SUM(2, 3);
    M4(result->M, 3, 0) = M4_SUM(3, 0);
    M4(result->M, 3, 1) = M4_SUM(3, 1);
    M4(result->M, 3, 2) = M4_SUM(3, 2);
    M4(result->M, 3, 3) = M4_SUM(3, 3);

#undef M4_SUM
}

void mat4_mul_vec4(vec4 *result, const mat4 *m, const vec4 *v) {

#define M4_SUM(row)  fixed_mul( M4(m->M, row, 0), v->p.x) + \
                     fixed_mul( M4(m->M, row, 1), v->p.y) + \
                     fixed_mul( M4(m->M, row, 2), v->p.z) + \
                     fixed_mul( M4(m->M, row, 3), v->p.w)

#define M4_SUM2(row) fixed_mul( M4(m->M, row, 0), v->p.x) + \
                     fixed_mul( M4(m->M, row, 1), v->p.y) + M4(m->M, row, 3)

#define M4_SUM3(row) fixed_mul( M4(m->M, row, 0), v->p.x) + \
                     fixed_mul( M4(m->M, row, 1), v->p.y) + \
                     fixed_mul( M4(m->M, row, 2), v->p.z) + M4(m->M, row, 3)

    if (v->p.w == fixed_int(1)) {
        if (v->p.z == fixed_int(0)) {
            result->p.x = M4_SUM2(0);
            result->p.y = M4_SUM2(1);
            result->p.z = M4_SUM2(2);
            result->p.w = M4_SUM2(3);
        } else {
            result->p.x = M4_SUM3(0);
            result->p.y = M4_SUM3(1);
            result->p.z = M4_SUM3(2);
            result->p.w = M4_SUM3(3);
        }
    } else {
        result->p.x = M4_SUM(0);
        result->p.y = M4_SUM(1);
        result->p.z = M4_SUM(2);
        result->p.w = M4_SUM(3);
    }

#undef M4_SUM
#undef M4_SUM2
#undef M4_SUM3
}

void mat4_identity(mat4 *m) {
    static fixed identity[16] = {
        fixed_int(1), fixed_int(0), fixed_int(0), fixed_int(0),
        fixed_int(0), fixed_int(1), fixed_int(0), fixed_int(0),
        fixed_int(0), fixed_int(0), fixed_int(1), fixed_int(0),
        fixed_int(0), fixed_int(0), fixed_int(0), fixed_int(1)
    };
    memcpy(m->M, identity, sizeof(fixed) * 16);
}
