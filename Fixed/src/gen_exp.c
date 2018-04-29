#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fixed.h"

int main(int argc, char **argv) {
    int i, j;

    FILE *f = fopen("exp.h", "w");

    if (f == NULL) return EXIT_FAILURE;

    fprintf(f, "static fixed ln_2 = %i;\n", fixed_double(log(2)));

    fprintf(f, "static fixed ln_table[] = {\n");
    j = 0;
    for (i = 0x0000;i <= 0xFF00;i += 0x0100) {
        if (i != 0) fprintf(f, "%i", fixed_double(log(fixed_todouble(i))));
        else fprintf(f, "%i", -1);

        if (i != 0xFF00) fprintf(f, ", ");
        else if (j != 15) fprintf(f, "\n");

        j++;
        if (j == 16) {
            fprintf(f, "\n");
            j = 0;
        }
    }
    fprintf(f, "};\n\n");

    fprintf(f, "static fixed exp_table[] = {\n");
    j = 0;
    for (i = 0;i <= 30;i++) {
        fixed r = fixed_double(exp(pow(2, i - 16)));
        if (r < 0) r = 0;
        fprintf(f, "%i", r);

        if (i != 30) fprintf(f, ", ");
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


