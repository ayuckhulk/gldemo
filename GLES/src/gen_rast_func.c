#include <stdio.h>
#include <stdlib.h>

#define MAX_FUNC (0x7F)

int main(int argc, char **argv) {
    int i;
    FILE *f = fopen("rast_func.h", "w");
    
    if (f == NULL) return EXIT_FAILURE;

    for (i = 0;i <= MAX_FUNC;i++) {
        fprintf(f, "void gl_DrawTriangle_%x(GLContext *context, Vertex *v0, Vertex *v1, Vertex *v2)\n", i);



        if (i & 0x40) {
        	if (i & 0x30) fprintf(f, "#define R_LOD\n");
        }
//    	fprintf(f, "#define MAX_VAR 7\n");
//    	fprintf(f, "#define MAX_LINEAR_VAR 5\n");
//    	fprintf(f, "#define VZ 0\n");
//    	fprintf(f, "#define VR 1\n");
//    	fprintf(f, "#define VG 2\n");
//    	fprintf(f, "#define VB 3\n");
//    	fprintf(f, "#define VA 4\n");
//    	fprintf(f, "#define VS 5\n");
//    	fprintf(f, "#define VT 6\n");
        switch((i & 0x30) >> 4) {
        case 0:
        	if ((i & 0x8) || ((i & 0x1))) {
            	fprintf(f, "#define MAX_VAR 5\n");
            	if (i & 0x4) fprintf(f, "#define MAX_LINEAR_VAR 4\n");
            	else fprintf(f, "#define MAX_LINEAR_VAR 5\n");
            	fprintf(f, "#define VA 4\n");
        	} else {
            	fprintf(f, "#define MAX_VAR 4\n");
            	if (i & 0x4) fprintf(f, "#define MAX_LINEAR_VAR 4\n");
            	else fprintf(f, "#define MAX_LINEAR_VAR 4\n");
        	}
        	fprintf(f, "#define VZ 0\n");
        	fprintf(f, "#define VR 1\n");
        	fprintf(f, "#define VG 2\n");
        	fprintf(f, "#define VB 3\n");

        	break;
        case 1:
        	fprintf(f, "#define R_TEXTURE0\n");
        	fprintf(f, "#define R_TEXTURE0_REPLACE\n");

        	fprintf(f, "#define MAX_VAR 3\n");
        	fprintf(f, "#define MAX_LINEAR_VAR 1\n");
        	fprintf(f, "#define VZ 0\n");
        	fprintf(f, "#define VS 1\n");
        	fprintf(f, "#define VT 2\n");

        	break;
        case 2:
        	fprintf(f, "#define R_TEXTURE0\n");
            fprintf(f, "#define R_TEXTURE0_MODULATE\n");

        	fprintf(f, "#define MAX_VAR 7\n");
        	fprintf(f, "#define MAX_LINEAR_VAR 5\n");
        	fprintf(f, "#define VZ 0\n");
        	fprintf(f, "#define VR 1\n");
        	fprintf(f, "#define VG 2\n");
        	fprintf(f, "#define VB 3\n");
        	fprintf(f, "#define VA 4\n");
        	fprintf(f, "#define VS 5\n");
        	fprintf(f, "#define VT 6\n");
            break;
        case 3:
//        	fprintf(f, "#define R_TEXTURE0\n");
//        	fprintf(f, "#define R_TEXTURE0_DECAL\n");
//
//        	fprintf(f, "#define MAX_VAR 7\n");
//        	fprintf(f, "#define MAX_LINEAR_VAR 5\n");
//        	fprintf(f, "#define VZ 0\n");
//        	fprintf(f, "#define VR 1\n");
//        	fprintf(f, "#define VG 2\n");
//        	fprintf(f, "#define VB 3\n");
//        	fprintf(f, "#define VA 4\n");
//        	fprintf(f, "#define VS 5\n");
//        	fprintf(f, "#define VT 6\n");

    		fprintf(f, "#define R_TEXTURE0\n");
    		fprintf(f, "#define R_TEXTURE0_MODULATE\n");
    		fprintf(f, "#define R_GREY_COLOR\n");

    		fprintf(f, "#define MAX_VAR 6\n");
    		fprintf(f, "#define MAX_LINEAR_VAR 3\n");
    		fprintf(f, "#define VZ 0\n");
    		fprintf(f, "#define VG 1\n");
    		fprintf(f, "#define VA 3\n");
    		fprintf(f, "#define VS 4\n");
    		fprintf(f, "#define VT 5\n");
        	break;
        default:
        	break;
        }
        if (i & 0x8) {
            fprintf(f, "#define R_BLEND\n");
        }
        if (i & 0x4) {
            fprintf(f, "#define R_PERSPECTIVE\n");
        } else {
            fprintf(f, "#define R_AFFINE\n");
        }
        if (i & 0x2) {
            fprintf(f, "#define R_DEPTH_TEST\n");
        }
        if (i & 0x1) {
            fprintf(f, "#define R_ALPHA_TEST\n");
        }

        fprintf(f, "#include \"triangle.h\"\n\n");
    }
    fprintf(f, "drawtriangle_func drawtriangle[] = {\n");
    for (i = 0;i <= MAX_FUNC;i++) {
        if (i != MAX_FUNC) fprintf(f, "    gl_DrawTriangle_%x,\n", i);
        else fprintf(f, "    gl_DrawTriangle_%x\n};\n", i);
    }
    fclose(f);
    return EXIT_SUCCESS;
}

