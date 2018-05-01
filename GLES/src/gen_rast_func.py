MAX_FUNC = 0x7F

for i in range(MAX_FUNC + 1):
    print("void gl_DrawTriangle_{:x}(GLContext *context, Vertex *v0, Vertex *v1, Vertex *v2)".format(i))
    if i & 0x40 != 0:
        if i & 0x30 != 0:
            print("#define R_LOD")
    v = (i & 0x30) >> 4
    if v == 0x0:
        if ((i & 0x8) != 0) or ((i & 0x1) != 0):
            print("#define MAX_VAR 5")
            if (i & 0x4) != 0:
                print("#define MAX_LINEAR_VAR 4")
            else:
                print("#define MAX_LINEAR_VAR 5")
            print("#define VA 4")
        else:
            print("#define MAX_VAR 4")
            if (i & 0x4) != 0:
                print("#define MAX_LINEAR_VAR 4")
            else:
                print("#define MAX_LINEAR_VAR 4") # TODO: duplicated code?

        print("#define VZ 0")
        print("#define VR 1")
        print("#define VG 2")
        print("#define VB 3")
    elif v == 0x1:
        print("#define R_TEXTURE0")
        print("#define R_TEXTURE0_REPLACE")

        print("#define MAX_VAR 3")
        print("#define MAX_LINEAR_VAR 1")
        print("#define VZ 0")
        print("#define VS 1")
        print("#define VT 2")
    elif v == 0x2:
        print("#define R_TEXTURE0")
        print("#define R_TEXTURE0_MODULATE")

        print("#define MAX_VAR 7")
        print("#define MAX_LINEAR_VAR 5")
        print("#define VZ 0")
        print("#define VR 1")
        print("#define VG 2")
        print("#define VB 3")
        print("#define VA 4")
        print("#define VS 5")
        print("#define VT 6")
    elif v == 0x3:
        print("#define R_TEXTURE0")
        print("#define R_TEXTURE0_MODULATE")
        print("#define R_GREY_COLOR")

        print("#define MAX_VAR 6")
        print("#define MAX_LINEAR_VAR 3")
        print("#define VZ 0")
        print("#define VG 1")
        print("#define VA 3")
        print("#define VS 4")
        print("#define VT 5")

    if (i & 0x8) != 0:
        print("#define R_BLEND")
    if (i & 0x4) != 0:
        print("#define R_PERSPECTIVE")
    else:
        print("#define R_AFFINE")
    if (i & 0x2) != 0:
        print("#define R_DEPTH_TEST")

    if (i & 0x1) != 0:
        print("#define R_ALPHA_TEST")

    print('#include "triangle.h"')
    print()

print("drawtriangle_func drawtriangle[] = {")
for i in range(MAX_FUNC + 1):
    end = ',\n' if i != MAX_FUNC else '\n};'
    print("    gl_DrawTriangle_{:x}".format(i), end=end)
