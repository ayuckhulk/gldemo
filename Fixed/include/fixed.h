#ifndef __fixed_h_
#define __fixed_h_

#include <limits.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
//#include "SDL/SDL.h"

#define FIXED_SHIFT 16

#define FIXED_INT_BITS 0xFFFF0000
#define FIXED_FRAC_BITS 0x0000FFFF

typedef int32_t fixed;

uint64_t divide32(uint32_t y, uint32_t x);

#define fixed_round(x) (((x) + (1 << (FIXED_SHIFT - 1))) & FIXED_INT_BITS)
#define fixed_toint_round(x) (((x) + (1 << (FIXED_SHIFT - 1))) >> FIXED_SHIFT)

#define fixed_fraq(x) ((x) & FIXED_FRAC_BITS)

#define fixed_int(x) ((fixed)((x) << FIXED_SHIFT))
#define fixed_float(x) ((fixed)((x) * (1 << FIXED_SHIFT)))
#define fixed_double(x) ((fixed)((x) * (1 << FIXED_SHIFT)))
#define fixed_toint(x) (((fixed)(x)) >> FIXED_SHIFT)
#define fixed_tofloat(x) ((float) (x) / (1 << FIXED_SHIFT))
#define fixed_todouble(x) ((double)(x) / (1 << FIXED_SHIFT))

#define fixed_mul(x, y) (( ((int64_t)(x)) * (y) ) >> FIXED_SHIFT)
#define fixed_div(x, y) ((fixed)((((int64_t)(x)) << FIXED_SHIFT) / (y)))
#define fixed_muldiv(a, x, y) ((fixed)(((((int64_t)(x)) * (a)) << FIXED_SHIFT) / (y)))

//fixed fixed_mul(fixed x, fixed y);
//fixed fixed_div(fixed x, fixed y);
fixed fixed_abs(fixed x);
fixed fixed_cos(fixed angle);
fixed fixed_sin(fixed angle);
fixed fixed_acos(fixed value);
fixed fixed_asin(fixed value);
fixed fixed_sqrt(fixed x);
fixed fixed_pow(fixed x, fixed y);
fixed fixed_ln(fixed x);
fixed fixed_exp(fixed x);

fixed fixed_interpolate(fixed x0, fixed x1, fixed t);

#endif

