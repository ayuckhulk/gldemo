cmake_minimum_required(VERSION 3.5)

project(glmath LANGUAGES C)

add_library(glmath STATIC
    src/vec3.c
    src/vec4.c
    src/mat3.c
    src/mat4.c
)

add_library(gl::math ALIAS glmath)

target_include_directories(glmath PUBLIC include)
target_link_libraries(glmath PUBLIC gl::fixed)
