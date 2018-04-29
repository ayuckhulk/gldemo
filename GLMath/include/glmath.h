#ifndef _GLMATH_H
#define _GLMATH_H

#include "fixed.h"

#define M3(m, row, col) ((m)[(row) + 3 * (col)])
#define M4(m, row, col) ((m)[(row) + 4 * (col)])

typedef union {
    struct {
        fixed x;
        fixed y;
        fixed z;
    } p;
    fixed xyz[3];
} vec3;

typedef union {
    struct {
        fixed x;
        fixed y;
        fixed z;
        fixed w;
    } p;
    fixed xyzw[4];
} vec4;

typedef struct {
    fixed M[9];
} mat3;

typedef struct {
    fixed M[16];
} mat4;

void vec3_from_vec4(vec3 *result, vec4 *v);
void vec3_set(vec3 *result, fixed x, fixed y, fixed z);
void vec3_inverse(vec3 *result, vec3 *v);
void vec3_scale(vec3 *result, vec3 *v, fixed a);
void vec3_add(vec3 *result, vec3 *v1, vec3 *v2);
void vec3_sub(vec3 *result, vec3 *v2, vec3 *v1);
void vec3_mul(vec3 *result, vec3 *v1, vec3 *v2);
fixed vec3_dot(vec3 *v1, vec3 *v2);
void vec3_cross(vec3 *result, const vec3 *v1, const vec3 *v2);
fixed vec3_length(vec3 *v);
fixed vec3_sqr_length(vec3 *v);
void vec3_normalize(vec3 *result, vec3 *v);
void vec3_mul_mat3(vec3 *result, const vec3 *v, const mat3 *m);

void vec4_set(vec4 *result, fixed x, fixed y, fixed z, fixed w);
void vec4_from_vec3(vec4 *result, vec3 *v);
//void vec4_mul_mat4(vec4 *result, vec4 *v, mat4 *m);

void mat3_from_mat4(mat3 *result, const mat4 *m);
fixed mat3_determinant(const mat3 *m);
void mat3_inverse(mat3 *result, const mat3 *m);
void mat3_mul_vec3(vec3 *result, const mat3 *m, const vec3 *v);

void mat4_from_array(mat4 *result, const fixed *array);
void mat4_copy(mat4 *dst, const mat4 *src);
void mat4_mul(mat4 *result, const mat4 *m1, const mat4 *m2);
void mat4_mul_vec4(vec4 *result, const mat4 *m, const vec4 *v);
void mat4_identity(mat4 *m);
//void mat4_create_translation(mat4 *m, fixed x, fixed y, fixed z);
//void mat4_create_scale(mat4 *m, GLfixed x, GLfixed y, GLfixed z);
//void mat4_create_rotation(mat4 *m, fixed angle, fixed x, fixed y, fixed z);
//void mat4_create_perspective(mat4 *m, fixed fovy, fixed aspect, fixed zNear, fixed zFar);
//void mat4_create_lookat(mat4 *m, fixed eyex, fixed eyey, fixed eyez,
//                        fixed centerx, fixed centery, fixed centerz,
//                        fixed upx, fixed upy, fixed upz);



#endif
