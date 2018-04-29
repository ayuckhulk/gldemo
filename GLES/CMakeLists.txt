cmake_minimum_required(VERSION 3.5)

project(GLES_CL LANGUAGES C)

add_executable(gen_div_table src/gen_div_table.c)
target_include_directories(gen_div_table PRIVATE include)
target_link_libraries(gen_div_table PRIVATE gl::fixed)

add_custom_command(
    OUTPUT ${GLES_CL_BINARY_DIR}/div_table.h
    COMMAND $<TARGET_FILE:gen_div_table>
    DEPENDS $<TARGET_FILE:gen_div_table>
)

add_executable(gen_rast_func src/gen_rast_func.c)
target_include_directories(gen_rast_func PRIVATE include src)

add_custom_command(
    OUTPUT ${GLES_CL_BINARY_DIR}/rast_func.h
    COMMAND $<TARGET_FILE:gen_rast_func>
    DEPENDS $<TARGET_FILE:gen_rast_func>
)

add_library(GLES_CL STATIC
    src/render.c
    src/stack.c
    src/context.c
    src/state.c
    src/arrays.c
    src/matrix.c
    src/list.c
    src/lights.c
    src/rasterizer.c
    src/texture.c

    ${GLES_CL_BINARY_DIR}/rast_func.h
    ${GLES_CL_BINARY_DIR}/div_table.h
)

add_library(gl::gles ALIAS GLES_CL)

target_include_directories(GLES_CL PRIVATE src ${GLES_CL_BINARY_DIR} include/GLES PUBLIC include)
target_link_libraries(GLES_CL PUBLIC gl::fixed gl::math)
