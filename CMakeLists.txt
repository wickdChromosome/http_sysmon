cmake_minimum_required(VERSION 2.8.12)
project(fuggoff)

add_definitions("-std=c++11")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(fuggoff main.cpp)
target_link_libraries(fuggoff ${CONAN_LIBS})
