cmake_minimum_required(VERSION 3.10)
project(CustomCompiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src/include)

file(GLOB SOURCES "src/*.cpp")

add_executable(mycc ${SOURCES})
