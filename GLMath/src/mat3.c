#include "glmath.h"

void mat3_from_mat4(mat3 *result, const mat4 *m) {
    M3(result->M, 0, 0) = M4(m->M, 0, 0);
    M3(result->M, 0, 1) = M4(m->M, 0, 1);
    M3(result->M, 0, 2) = M4(m->M, 0, 2);

    M3(result->M, 1, 0) = M4(m->M, 1, 0);
    M3(result->M, 1, 1) = M4(m->M, 1, 1);
    M3(result->M, 1, 2) = M4(m->M, 1, 2);

    M3(result->M, 2, 0) = M4(m->M, 2, 0);
    M3(result->M, 2, 1) = M4(m->M, 2, 1);
    M3(result->M, 2, 2) = M4(m->M, 2, 2);
}

static int mat3_min[3][2] = {{1, 2}, {0, 2}, {0, 1}};

#define M3_MINOR(m, i, j) ( fixed_mul( M3(m->M, mat3_min[i][0], mat3_min[j][0]), M3(m->M, mat3_min[i][1], mat3_min[j][1]) ) - \
                          fixed_mul( M3(m->M, mat3_min[i][0], mat3_min[j][1]), M3(m->M, mat3_min[i][1], mat3_min[j][0]) ) )

#define M3_SIGN_MINOR(i, j, m) ((((i) + (j)) & 0x1) ? (-(M3_MINOR(m, i, j))) : (M3_MINOR(m, i, j)))

fixed mat3_determinant(const mat3 *m) {
    return fixed_mul(M3(m->M, 0, 0), M3_MINOR(m, 0, 0)) -
           fixed_mul(M3(m->M, 0, 1), M3_MINOR(m, 0, 1)) +
           fixed_mul(M3(m->M, 0, 2), M3_MINOR(m, 0, 2));
}

void mat3_inverse(mat3 *result, const mat3 *m) {
    fixed det;

    det = mat3_determinant(m);

    if (det == 0) return;

    det = fixed_div(fixed_int(1), det);

    M3(result->M, 0, 0) = fixed_mul(M3_SIGN_MINOR(0, 0, m), det);
    M3(result->M, 0, 1) = fixed_mul(M3_SIGN_MINOR(1, 0, m), det);
    M3(result->M, 0, 2) = fixed_mul(M3_SIGN_MINOR(2, 0, m), det);
    M3(result->M, 1, 0) = fixed_mul(M3_SIGN_MINOR(0, 1, m), det);
    M3(result->M, 1, 1) = fixed_mul(M3_SIGN_MINOR(1, 1, m), det);
    M3(result->M, 1, 2) = fixed_mul(M3_SIGN_MINOR(2, 1, m), det);
    M3(result->M, 2, 0) = fixed_mul(M3_SIGN_MINOR(0, 2, m), det);
    M3(result->M, 2, 1) = fixed_mul(M3_SIGN_MINOR(1, 2, m), det);
    M3(result->M, 2, 2) = fixed_mul(M3_SIGN_MINOR(2, 2, m), det);
}

void mat3_mul_vec3(vec3 *result, const mat3 *m, const vec3 *v) {
#define M3_SUM(row) fixed_mul( M3(m->M, row, 0), v->p.x) + \
                    fixed_mul( M3(m->M, row, 1), v->p.y) + \
                    fixed_mul( M3(m->M, row, 2), v->p.z)

    result->p.x = M3_SUM(0);
    result->p.y = M3_SUM(1);
    result->p.z = M3_SUM(2);

#undef M3_SUM
}

