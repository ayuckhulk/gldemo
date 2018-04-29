#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fixed.h"

#define COS_BITS 12

#define COS_TABLE_SIZE (1 << (COS_BITS - 1))

static fixed cos_table[COS_TABLE_SIZE];

void compute_cos(void) {
    int i;
    for (i = 0;i < COS_TABLE_SIZE;i++) {
        cos_table[i] = fixed_double(cos((double) i * M_PI / (double) COS_TABLE_SIZE));
    }
}

int main(int argc, char **argv) {
    int i, j;

    FILE *f = fopen("cos.h", "w");

    if (f == NULL) return EXIT_FAILURE;

    compute_cos();

    fprintf(f, "#define COS_BITS %i\n", COS_BITS);
    fprintf(f, "#define ANGLE_CLAMP ((1 << COS_BITS) - 1)\n");
    fprintf(f, "#define COS_TABLE_SIZE (1 << (COS_BITS - 1))\n");
    fprintf(f, "#define DEGTOX fixed_double(((double) ANGLE_CLAMP / (double) 359))\n\n");
    fprintf(f, "static fixed cos_table[COS_TABLE_SIZE] = {\n");
    j = 0;
    for (i = 0;i < COS_TABLE_SIZE;i++) {
        fprintf(f, "%i", cos_table[i]);
        
        if (i != (COS_TABLE_SIZE - 1)) fprintf(f, ", ");
        else if (j != 15) fprintf(f, "\n");

        j++;
        if (j == 16) {
            fprintf(f, "\n");
            j = 0;
        }
    }
    fprintf(f, "};\n");

    fclose(f);

    return EXIT_SUCCESS;
}


