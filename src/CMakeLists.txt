cmake_minimum_required(VERSION 2.8.12)
project(project)

link_libraries(stdc++fs)

add_definitions("-std=c++17")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(project main.cpp)
target_link_libraries(project ${CONAN_LIBS})
