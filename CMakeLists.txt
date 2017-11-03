cmake_minimum_required(VERSION 3.5)

enable_language(CXX)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)

find_package(Threads REQUIRED)
	if(CMAKE_USE_PTHREADS_INIT)
	    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
	endif()
find_package(Boost REQUIRED)

add_definitions(-std=c++11 -pthread)

set(EXE main.cpp)
set(LIB image.cpp)

add_library(image STATIC ${LIB})
add_executable(main ${EXE})
add_subdirectory(test)

enable_testing()

target_link_libraries(main image ${CMAKE_THREAD_LIBS_INIT})
