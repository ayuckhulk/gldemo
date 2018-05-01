#include "common.h"

MatrixStack *MatrixStack_Create(GLsizei size) {
    MatrixStack *stack;

    stack = (MatrixStack *) malloc(sizeof(MatrixStack));
    if (stack == NULL) return NULL;

    stack->mat = (mat4 *) malloc(size * sizeof(mat4));
    if (stack->mat == NULL) {
        free(stack);
        return NULL;
    }
    stack->size = size;
    stack->depth = 1;
    MatrixStack_LoadIdentity(stack);

    stack->modified = GL_TRUE;

    return stack;
}

void MatrixStack_Destroy(MatrixStack *stack) {
    free(stack->mat);
    free(stack);
}

GLboolean MatrixStack_Push(MatrixStack *stack) {
    mat4 *address;
    if (stack->depth >= stack->size) return GL_FALSE;
    address = stack->mat + stack->depth;
    stack->depth++;
    mat4_copy(address, address - 1);
    return GL_TRUE;
}

GLboolean MatrixStack_Pop(MatrixStack *stack) {
    if (stack->depth <= 1) return GL_FALSE;
    stack->depth--;
    stack->identity = GL_FALSE;
    stack->modified = GL_TRUE;
    return GL_TRUE;
}

void MatrixStack_LoadIdentity(MatrixStack *stack) {
    mat4_identity(MatrixStack_GetCurrent(stack));
    stack->identity = GL_TRUE;
    stack->modified = GL_TRUE;
}

mat4 *MatrixStack_GetCurrent(MatrixStack *stack) {
    return stack->mat + (stack->depth - 1);
}

void MatrixStack_Mult(MatrixStack *lstack, MatrixStack *rstack, mat4 *result) {
    if (lstack->identity) {
        mat4_copy(result, MatrixStack_GetCurrent(rstack));
    } else if (rstack->identity) {
        mat4_copy(result, MatrixStack_GetCurrent(lstack));
    } else {
        mat4_mul(result, MatrixStack_GetCurrent(lstack), MatrixStack_GetCurrent(rstack));
    }
}

void MatrixStack_MultMatrix(MatrixStack *stack, const mat4 *m) {
    mat4 new_matrix;
    mat4 *current_matrix = MatrixStack_GetCurrent(stack);

    if (stack->identity) {
        mat4_copy(current_matrix, m);
    } else {
        mat4_mul(&new_matrix, current_matrix, m);
        mat4_copy(current_matrix, &new_matrix);
    }
    stack->identity = GL_FALSE;
    stack->modified = GL_TRUE;
}

GLboolean MatrixStack_IsModified(MatrixStack *stack) {
    return stack->modified;
}

void MatrixStack_SetModified(MatrixStack *stack, GLboolean modified) {
    stack->modified = modified;
}
