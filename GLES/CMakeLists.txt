cmake_minimum_required(VERSION 3.5)

project(GLES_CL LANGUAGES C)

find_package(PythonInterp 3 REQUIRED)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/div_table.h
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/gen_div_table.py > ${CMAKE_CURRENT_BINARY_DIR}/div_table.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/gen_div_table.py
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rast_func.h
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/gen_rast_func.py > ${CMAKE_CURRENT_BINARY_DIR}/rast_func.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/gen_rast_func.py
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
