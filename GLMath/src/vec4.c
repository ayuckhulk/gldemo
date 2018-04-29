#include "glmath.h"

void vec4_from_vec3(vec4 *result, vec3 *v) {
    result->p.x = v->p.x;
    result->p.y = v->p.y;
    result->p.z = v->p.z;
    result->p.w = fixed_int(1);
}

void vec4_set(vec4 *result, fixed x, fixed y, fixed z, fixed w) {
    result->p.x = x;
    result->p.y = y;
    result->p.z = z;
    result->p.w = w;
}
