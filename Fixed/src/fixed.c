#include "fixed.h"

#ifndef fixed_mul

inline fixed fixed_mul(fixed x, fixed y) {
    register uint32_t xi, xf;
    register uint32_t yi, yf;
    register unsigned char minus = 0;

    if (x < 0) {
        x = -x;
        minus = 1;
    }
    if (y < 0) {
        y = -y;
        minus = 1 - minus;
    }

    xi = xtoi16(x & FIXED_INT_BITS);
    xf = x & FIXED_FRAC_BITS;
    yi = xtoi16(y & FIXED_INT_BITS);
    yf = y & FIXED_FRAC_BITS;

    xi = ((xi * yi) << FIXED_SHIFT) + xi * yf + yi * xf + ((xf * yf) >> FIXED_SHIFT);

    if (minus) return -xi;
    else return xi;
}

#endif

inline fixed fixed_abs(fixed x) {
    return ((x) >= 0 ? (x) : -(x));
}

#include "cos.h"

fixed fixed_cos(fixed angle) {
    int index = fixed_toint(fixed_mul(angle, DEGTOX)) & ANGLE_CLAMP;
    if (index >= COS_TABLE_SIZE) return -cos_table[index & (ANGLE_CLAMP >> 1)];
    else return cos_table[index];
}

fixed fixed_sin(fixed angle) {
    return fixed_cos(fixed_int(90) - angle);
}

fixed fixed_acos(fixed x) {
    int i = 0;
    int j = COS_TABLE_SIZE - 1;
    int m;
    while (i != j) {
        m = (i + j) >> 1;
        if (cos_table[m] > x) i = m + 1;
        else j = m;
    }
    return (fixed_int(180) >> (COS_BITS - 1)) * i;
}

fixed fixed_asin(fixed x) {
    return fixed_int(90) - fixed_acos(x);
}

fixed fixed_sqrt(fixed x) {
    //assert(x >= 0);

    fixed result = 0;
    int group;
    int sum = 0;
    int diff = 0;

    if (x <= 0) return 0;

    for (group = 15;group >= 0;group--)	{
        sum <<= 1;
        sum++;

        diff <<= 2;
        diff += (x >> (group << 1)) & 0x3;

        if (sum <= diff) {
            result += (1 << group);
            diff -= sum;
            sum++;
        } else sum-=1;
    }
    return result << 8;
}

fixed fixed_pow(fixed x, fixed y) {
    if (x == 0) return 0;
    return fixed_exp(fixed_mul(y, fixed_ln(x)));
}

#include "exp.h"

fixed fixed_ln(fixed x) {
    int p = 0;
    if (x == 0) return 0;
    if (x > 0x0000FFFF) {
        while (x & 0xFFFF0000) {
            x >>= 1;
            p++;
        }
    } else {
        while ((x & 0x8000) == 0) {
            x <<= 1;
            p--;
        }
    }
    return ln_table[(x & 0xFF00) >> 8] + p * ln_2;
}

fixed fixed_exp(fixed x) {
    fixed result = fixed_int(1);
    int i;
    int mask = 0x40000000;
    for (i = 30;i >= 0;i--) {
        mask >>= (30 - i);
        if (x & mask) {
            result = fixed_mul(result, exp_table[i]);
        }
    }
    return result;
}

uint64_t divide32(uint32_t y, uint32_t x) {
   uint16_t n;
   uint64_t answer;
   uint64_t remainder;
   uint64_t divisor;

   answer = 0;
   remainder = x;
   divisor = (uint64_t)y << 32;

   for (n = 0; n < 32; n++) {
      divisor = divisor >> 1;
      if (remainder >= divisor) {
         remainder -= divisor;
         answer |= (uint64_t)1 << (63 - n);
      }
   }

   for (n = 0; n < 32; n++) {
      remainder = remainder << 1;
      if (remainder >= divisor) {
         remainder -= divisor;
         answer |= (uint64_t)1 << (31 - n);
      }
   }

   return answer;
}

fixed fixed_interpolate(fixed x0, fixed x1, fixed t) {
    return x0 + fixed_mul(x1 - x0, t);
}
