cmake_minimum_required(VERSION 3.5)

project(roller LANGUAGES C)

find_package(SDL2 REQUIRED)

add_executable(roller WIN32
    main.c
    roller.c
    point.c
    pillar.c
)

if (WIN32)
    set(LINK_LIBS SDL2::SDL2 SDL2::SDL2main)
else()
    set(LINK_LIBS SDL2 SDL2main m)
endif()

target_link_libraries(roller PUBLIC gl::gles PRIVATE ${LINK_LIBS})
