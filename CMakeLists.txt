cmake_minimum_required(VERSION 3.5)

project(cube)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)

add_executable(cube cube.c)

target_link_libraries(cube PRIVATE SDL2::SDL2)
