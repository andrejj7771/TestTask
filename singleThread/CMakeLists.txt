cmake_minimum_required(VERSION 2.8)

project(TestTask)
add_definitions(-std=c++11)

set(EXE main.cpp)
set(LIB image.cpp)

add_library(image ${LIB})
add_executable(main ${EXE})

target_link_libraries(main image)
