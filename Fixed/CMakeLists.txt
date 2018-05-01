cmake_minimum_required(VERSION 3.5)

project(fixed LANGUAGES C)

find_package(PythonInterp 3 REQUIRED)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cos.h
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/gen_cos.py > ${CMAKE_CURRENT_BINARY_DIR}/cos.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/gen_cos.py
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/exp.h
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/gen_exp.py > ${CMAKE_CURRENT_BINARY_DIR}/exp.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/gen_exp.py
)

add_library(fixed STATIC src/fixed.c ${CMAKE_CURRENT_BINARY_DIR}/cos.h ${CMAKE_CURRENT_BINARY_DIR}/exp.h)
add_library(gl::fixed ALIAS fixed)

target_include_directories(fixed PRIVATE ${CMAKE_CURRENT_BINARY_DIR} PUBLIC include)
