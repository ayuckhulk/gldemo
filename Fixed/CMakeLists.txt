cmake_minimum_required(VERSION 3.5)

project(fixed LANGUAGES C)

add_executable(gen_cos src/gen_cos.c)
target_include_directories(gen_cos PRIVATE include)

add_custom_command(
    OUTPUT ${fixed_BINARY_DIR}/cos.h
    COMMAND $<TARGET_FILE:gen_cos>
    DEPENDS $<TARGET_FILE:gen_cos>
)

add_executable(gen_exp src/gen_exp.c)
target_include_directories(gen_exp PRIVATE include)

add_custom_command(
    OUTPUT ${fixed_BINARY_DIR}/exp.h
    COMMAND $<TARGET_FILE:gen_exp>
    DEPENDS $<TARGET_FILE:gen_exp>
)

add_library(fixed STATIC src/fixed.c ${fixed_BINARY_DIR}/cos.h ${fixed_BINARY_DIR}/exp.h)
add_library(gl::fixed ALIAS fixed)

target_include_directories(fixed PRIVATE ${fixed_BINARY_DIR} PUBLIC include)
