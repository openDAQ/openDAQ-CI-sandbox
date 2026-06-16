cmake_minimum_required(VERSION 3.16)
project(homebrew_clang_exception_test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(exception_test main.cpp)
