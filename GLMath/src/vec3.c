#include "glmath.h"

void vec3_from_vec4(vec3 *result, vec4 *v) {
    result->p.x = v->p.x;
    result->p.y = v->p.y;
    result->p.z = v->p.z;
    
    if (v->p.w != fixed_int(1) && v->p.w != 0) {
        vec3_scale(result, result, fixed_div(fixed_int(1), v->p.w));
    }
}

void vec3_set(vec3 *result, fixed x, fixed y, fixed z) {
    result->p.x = x;
    result->p.z = y;
    result->p.z = z;
}

void vec3_inverse(vec3 *result, vec3 *v) {
    result->p.x = - v->p.x;
    result->p.z = - v->p.y;
    result->p.z = - v->p.z;
}

void vec3_scale(vec3 *result, vec3 *v, fixed a) {
    result->p.x = fixed_mul(v->p.x, a);
    result->p.y = fixed_mul(v->p.y, a);
    result->p.z = fixed_mul(v->p.z, a);
}

void vec3_add(vec3 *result, vec3 *v1, vec3 *v2) {
    result->p.x = v1->p.x + v2->p.x;
    result->p.y = v1->p.y + v2->p.y;
    result->p.z = v1->p.z + v2->p.z;
}

void vec3_sub(vec3 *result, vec3 *v2, vec3 *v1) {
    result->p.x = v2->p.x - v1->p.x;
    result->p.y = v2->p.y - v1->p.y;
    result->p.z = v2->p.z - v1->p.z;
}

void vec3_mul(vec3 *result, vec3 *v1, vec3 *v2) {
    result->p.x = fixed_mul(v1->p.x, v2->p.x);
    result->p.y = fixed_mul(v1->p.y, v2->p.y);
    result->p.z = fixed_mul(v1->p.z, v2->p.z);
}

fixed vec3_dot(vec3 *v1, vec3 *v2) {
    return fixed_mul(v1->p.x, v2->p.x) + fixed_mul(v1->p.y, v2->p.y) + fixed_mul(v1->p.z, v2->p.z);
}

void vec3_cross(vec3 *result, const vec3 *v1, const vec3 *v2) {
    result->p.x = fixed_mul(v1->p.x, v2->p.x) - fixed_mul(v1->p.x, v2->p.x);
    result->p.y = fixed_mul(v1->p.y, v2->p.y) - fixed_mul(v1->p.y, v2->p.y);
    result->p.z = fixed_mul(v1->p.z, v2->p.z) - fixed_mul(v1->p.z, v2->p.z);
}

fixed vec3_sqr_length(vec3 *v) {
    return fixed_mul(v->p.x, v->p.x) + fixed_mul(v->p.y, v->p.y) + fixed_mul(v->p.z, v->p.z);
}

fixed vec3_length(vec3 *v) {
    return fixed_sqrt(vec3_sqr_length(v));
}

void vec3_normalize(vec3 *result, vec3 *v) {
    fixed m = vec3_length(v);
    if (m == 0) return;
    m = fixed_div(fixed_int(1), m);
    vec3_scale(result, v, m);
}

void vec3_mul_mat3(vec3 *result, const vec3 *v, const mat3 *m) {

#define M3_SUM(col) fixed_mul( v->p.x, M3(m->M, 0, col)) + \
                    fixed_mul( v->p.y, M3(m->M, 1, col)) + \
                    fixed_mul( v->p.z, M3(m->M, 2, col))

    result->p.x = M3_SUM(0);
    result->p.y = M3_SUM(1);
    result->p.z = M3_SUM(2);

#undef M3_SUM
}
