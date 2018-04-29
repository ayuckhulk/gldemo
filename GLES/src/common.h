#ifndef __common_h_
#define __common_h_

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "gl.h"
#include "fixed.h"
#include "glmath.h"

#define PIXEL unsigned short

#define MAX_MODELVIEW_STACK_DEPTH 16
#define MAX_TEXTURE_STACK_DEPTH 2
#define MAX_PROJECTION_STACK_DEPTH 4

#define ZBUFFER_DEPTH 10
#define ZBUFFER_SCALE ((1 << ZBUFFER_DEPTH) - 1)

#define MAX_TEXTURE_UNITS 1

#define TEXTURE0 0
#define TEXTURE1 1

#define MAX_TEXTURES 500
#define MAX_TEXTURE_SIZE 1024
#define MAX_LOD (10 + 1)

#define MAX_LIGHTS 8

#define MAX_VERTEX 16

#define MODELVIEW_MATRIX 0
#define PROJECTION_MATRIX 1
#define TEXTURE_MATRIX 2

// Rendering structures
typedef struct {
    GLubyte r;
    GLubyte g;
    GLubyte b;
    GLubyte a;
} Color;

typedef struct {
    vec4 v;
    vec4 t[MAX_TEXTURE_UNITS];
    GLubyte color[4];
} Vertex;

typedef struct {
    vec4 v;
    vec3 n;
} VertexData;

typedef struct {
    GLint count;

    GLint last;
    GLint next[MAX_VERTEX + 2];
    Vertex vertex[MAX_VERTEX + 2];
    GLfixed clip[MAX_VERTEX + 2];

    GLboolean loop;
} VertexList;

typedef struct {
    VertexList list;
    GLenum mode;
    GLint n;

    Vertex vertex[4];
    VertexData data[4];

    GLuint v0, v1, v2;

    GLint index;
    GLboolean data_locked;
} RenderingBuffer;

// Matrix stack
typedef struct {
    GLsizei size;
    mat4 *mat;
    GLint depth;
    GLboolean identity;
    GLboolean modified;
} MatrixStack;

// Texture

typedef struct TextureArray TextureArray;

struct TextureArray {
    PIXEL *texels;
    GLubyte *alpha;

    GLsizei width;
    GLsizei height;

    GLint s_mask;
    GLint t_mask;
    GLsizei t_shift;
};

typedef struct Texture Texture;

struct Texture {
    GLuint id;
    Texture *prev;
    Texture *next;

    TextureArray lod[MAX_LOD];
    int max_lod;

    GLint min_filter;
    GLint mag_filter;
};

// Client arrays
typedef struct {
    GLboolean enabled;

    GLubyte *base;
    GLubyte *current;

    GLint size;
    GLenum type;
    GLsizei stride;
} Array;

// Texture unit
typedef struct {
    GLboolean enabled;
    MatrixStack *matrix;
    Texture *texture;

    vec4 current_texcoord;
    Array texcoord;
    GLint scales, scalet;

    GLint env_mode;
} TextureUnit;

// Light
typedef struct {
    GLboolean enabled;

    GLfixed ac[4];
    GLfixed dc[4];
    GLfixed sc[4];
    vec4 P;
    vec3 sd;
    GLfixed sr;
    GLfixed cr;
    GLfixed k0;
    GLfixed k1;
    GLfixed k2;
} Light;

typedef struct {
    // Error handling
    GLenum error;

    // Frame buffer
    PIXEL *framebuffer;
    PIXEL clear_color;

    GLsizei width;
    GLsizei height;
    GLsizei pitch;

    // Z Buffer
    GLfixed *zbuffer;
    GLfixed clear_depth;

    GLclampx znear;
    GLclampx zfar;

    GLboolean depth_test_enabled;

    // Alpha test
    GLboolean alpha_test_enabled;
    GLint alpha_value;

    // Viewport
    GLint view_x;
    GLint view_y;
    GLsizei view_width;
    GLsizei view_height;
    //Scissor
    GLfixed xmin;
    GLfixed xmax;
    GLfixed ymin;
    GLfixed ymax;

    // Matrixes
    GLenum matrix_mode;
    MatrixStack *matrix[3];
    mat4 transform_matrix;

    // Textures
    Texture *textures[MAX_TEXTURES];
    TextureUnit *TU[MAX_TEXTURE_UNITS];
    GLint texture_unit;
    GLint client_texture_unit;

    // Alpha
    GLboolean blend_enabled;

    // Arrays
    Array vertex;

    // Color
    GLubyte current_color[4];
    Array color;
    GLenum shade_model;
    GLubyte flat_color[4];
    GLboolean color_material_enabled;
    GLboolean grey_color;

    // Normals
    Array normal;
    vec3 current_normal;
    GLboolean normalize_enabled;
    GLboolean rescale_normal_enabled;
    GLfixed rescale_normal;
    mat3 normal_transform;

    // Lighting
    GLboolean lighting_enabled;
    Light lights[MAX_LIGHTS];
    GLboolean update_lighting;

    // Lighting model
    GLfixed acs[4];
    GLboolean tbs;

    //Material
    GLfixed acm[4];
    GLfixed dcm[4];
    GLfixed scm[4];
    GLfixed ecm[4];
    GLfixed srm;

    // Culling
    GLboolean cull_face_enabled;
    GLenum cull_face_mode;
    GLenum front_face;

    //Pixel store parameters
    GLint unpack_alignment;

    // Rendering
    RenderingBuffer rendering_buffer;

    // Rasterizer
    //GLfixed *div_table;

    GLfixed line_width;
    GLfixed point_size;

    GLboolean perspective_correction;
} GLContext;

typedef void (*drawtriangle_func)(GLContext *context, Vertex *v0, Vertex *v1, Vertex *v2);
typedef void (*drawline_func)(GLContext *context, Vertex *v0, Vertex *v1);
typedef void (*drawpoint_func)(GLContext *context, Vertex *v);

#define RGB_TO_PIXEL(r,g,b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3))

// 0%      25%     50%     75%     100%
// |-------|-------|-------|-------|
// ----|-------|-------|-------|----
// |---|---|---|---|---|---|---|---|
//   64: >> 6

#define toAlpha(a) ((((GLuint)(a)) + 1) >> 3)
GLContext *gl_GetCurrentContext(void);

void gl_SetError(GLContext *context, GLenum error);
GLenum gl_GetError(GLContext *context);

// Matrices
//void gl_CopyMatrixx(const GLfixed *src, GLfixed *dst);
//void gl_MultMatrixx(GLfixed *result, const GLfixed *c, const GLfixed *m);
//void gl_MultVectorx(GLfixed *result, const GLfixed *m, const GLfixed *v);
//void gl_MultTexCoordx(GLfixed *result, const GLfixed *m, const GLfixed *v);
//void gl_MultDirectionx(GLfixed *result, const GLfixed *m, const GLfixed *d);
//void gl_MultNormalx(GLfixed *result, const GLfixed *n, const GLfixed *m);
//void gl_LoadIdentityx(GLfixed *c);

//void gl_CopyMatrixf(const GLfloat *src, GLfloat *dst);
//void gl_CopyMatrixxf(const GLfixed *src, GLfloat *dst);
//void gl_CopyMatrixfx(const GLfloat *src, GLfixed *dst);

//void gl_GetNormalTransformx(GLfixed *result, GLfixed *m4);
//void gl_RescaleNormalx(GLfixed *result, GLfixed f, GLfixed *n);
//void gl_Normalizex(GLfixed *result, GLfixed *n);

// Textures
Texture *Texture_Create(GLuint id);
void Texture_Destroy(Texture *texture);
GLboolean Texture_IsComplete(Texture *texture);

TextureUnit *TextureUnit_Create(void);
void TextureUnit_Destroy(TextureUnit *unit);

int log2i(int i);

// Vertex list
void VertexList_Clear(VertexList *list);
int VertexList_First(VertexList *list);
int VertexList_Next(VertexList *list, int index);
Vertex *VertexList_Append(VertexList *list, Vertex *v);
GLboolean VertexList_Clip(VertexList *list);
GLboolean VertexList_BackFace(VertexList *list);

// Rendering buffer
void RenderingBuffer_Begin(RenderingBuffer *buffer, GLenum mode);
void RenderingBuffer_End(RenderingBuffer *buffer);
Vertex *RenderingBuffer_CurrentVertex(RenderingBuffer *buffer);
VertexData *RenderingBuffer_CurrentVertexData(RenderingBuffer *buffer);
void RenderingBuffer_ProcessVertex(RenderingBuffer *buffer);
void RenderingBuffer_RenderPoint(RenderingBuffer *buffer);
void RenderingBuffer_RenderLine(RenderingBuffer *buffer);
void RenderingBuffer_RenderTriangle(RenderingBuffer *buffer);

// Matrix stack
MatrixStack *MatrixStack_Create(GLsizei size);
void MatrixStack_Destroy(MatrixStack *stack);
void MatrixStack_LoadIdentity(MatrixStack *stack);
GLboolean MatrixStack_Push(MatrixStack *stack);
GLboolean MatrixStack_Pop(MatrixStack *stack);
void MatrixStack_Mult(MatrixStack *lstack, MatrixStack *rstack, mat4 *result);
void MatrixStack_MultMatrix(MatrixStack *stack, const mat4 *m);
mat4 *MatrixStack_GetCurrent(MatrixStack *stack);
GLboolean MatrixStack_IsModified(MatrixStack *stack);
void MatrixStack_SetModified(MatrixStack *stack, GLboolean modified);

// Arrays
void Array_Init(Array *array);
void Array_Set(Array *array, GLint size, GLenum type, GLsizei stride,
        const GLvoid *pointer);
void Array_Get(Array *array, GLuint index, GLfixed *matrix, GLfixed *v,
        GLint size);

// Lights
void Lights_Init(Light *lights);
void Ligths_ComputeColor(Light *lights, Vertex *vertex, VertexData *data, GLboolean back);
void rgba_set(GLfixed *rgba, GLfixed r, GLfixed g, GLfixed b, GLfixed a);

// ...
//void gl_ProjectVertex(GLContext *context, Vertex *v);
//void gl_TransformVectorx(GLfixed *src, GLsizei src_size, GLfixed *m,
//        GLfixed *dst, GLsizei dst_size);
void gl_Render(GLContext *context, VertexList *list);
extern drawtriangle_func drawtriangle[];

#endif /* __common_h_ */
