#include "common.h"
#include "modulate.h"
#include "div_table.h"

#define X 0
#define Y 1
#define Z 2
#define W 3

#define S 0
#define T 1

#define R 0
#define G 1
#define B 2
#define A 3

//#define MAX_VAR 7
//#define MAX_LINEAR_VAR 5
//#define VZ 0
//#define VR 1
//#define VG 2
//#define VB 3
//#define VA 4
//#define VS 5
//#define VT 6


#define COLOR_SHIFT 12
#define MIN_PERSPECTIVE 16
#define R_W_SHIFT 6

#define MAX(du, dv) ((du) > (dv) ? (du) : (dv))
#define LOD_MAX(du, dv) MAX(fixed_abs(du), fixed_abs(dv))

#define TEXTURE2D_OFFSET (((var_v[VT] & t0_t_mask) >> t0_t_shift) | ((var_v[VS] & t0_s_mask) >> FIXED_SHIFT))
#define TEXTURE2D_256 (((var_v[VT] >> 8) & 0xFF00) | ((var_v[VS] >> 16) & 0xFF))

// GL_LESS (default)
#define R_DEPTH_FUNC(depth) ((depth) < *zbuffer)

// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

#define MODULATE_COLOR(c) (modulate_5b[(((var_v[VB]) & 0xF80000) >> 14) | ((c) & 0x1F)] | \
                          modulate_6g[(((var_v[VG]) & 0xFC0000) >> 12) | (((c) & 0x07E0) >> 5)] | \
                          modulate_5r[(((var_v[VR]) & 0xF80000) >> 14) | (((c) & 0xF800) >> 11)])

#define MODULATE_ALPHA(a) ((toAlpha(fixed_toint(var_v[VA]))*a) >> 5)



//GLfixed Stri(GLfixed x0, GLfixed y0, GLfixed x1, GLfixed y1, GLfixed x2, GLfixed y2) {
//    GLfixed s;
//    s = fixed_mul(x1 - x0, y2 - y0) - fixed_mul(x2 - x0, y1 - y0);
//    if (s < 0) s = -s;
//    return s >> 1;
//}

/*
void gl_FillTriangle(GLContext *context, Vertex *v0, Vertex *v1, Vertex *v2) {
#include "rasterizer.h"
}
*/

#include "rast_func.h"

#undef X
#undef Y
#undef Z
#undef S
#undef T
