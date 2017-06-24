cmake_minimum_required(VERSION 2.8)

project(console C)

include_directories(include)

file(GLOB SRC "src/*.c")

set(VERBOSE 1)

add_executable(console ${SRC})
#cmake_verbose_makefile(on)
