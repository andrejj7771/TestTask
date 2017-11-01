cmake_minimum_required(VERSION 3.1)

enable_language(CXX)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
endif()

project(TestTask)
add_definitions(-std=c++11 -pthread)

set(EXE main.cpp)
set(LIB image.cpp)

add_library(image ${LIB})
add_executable(main ${EXE})

target_link_libraries(main image ${CMAKE_THREAD_LIBS_INIT})
