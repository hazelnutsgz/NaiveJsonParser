cmake_minimum_required(VERSION 3.10)
project(json_parser C)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
endif()

add_library(naivejson naivejson.c)
add_executable(naivejson_test test.c)
target_link_libraries(naivejson_test naivejson)