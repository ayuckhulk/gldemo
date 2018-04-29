#include <stdio.h>
#include <stdlib.h>
#include "fixed.h"

#define MAX_DIV 2048

int main(int argc, char **argv) {
    int i;
    FILE *f = fopen("div_table.h", "w");

    if (f == NULL) return EXIT_FAILURE;

    fprintf(f, "GLfixed div_table[] = {\n");

    fprintf(f, "%i, ", 0);
    for (i = 1;i < MAX_DIV;i++) {

    	fprintf(f, "%i", fixed_float((1.0f / (float)i)));

    	if (i == (MAX_DIV - 1)) fprintf(f, " \n};\n");
    	else fprintf(f, ", ");
    	if (i % 32 == 0) fprintf(f, "\n");
    }
    fclose(f);
    return EXIT_SUCCESS;
}
