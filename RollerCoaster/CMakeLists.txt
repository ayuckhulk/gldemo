cmake_minimum_required(VERSION 3.5)

project(roller LANGUAGES C)

find_package(SDL2 REQUIRED)

add_executable(roller
    main.c
    roller.c
    point.c
    pillar.c
)

target_include_directories(roller PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(roller PUBLIC gl::gles PRIVATE ${SDL2_LIBRARIES})
