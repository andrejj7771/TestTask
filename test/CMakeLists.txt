project(tests)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
        if(CMAKE_USE_PTHREADS_INIT)
            set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
        endif()

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

set(test_src test.cpp)

add_executable(tests ${test_src})

target_link_libraries(tests image ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_test(NAME conversionTest COMMAND "/usr/bin/python" "-t" "runConvTest")
