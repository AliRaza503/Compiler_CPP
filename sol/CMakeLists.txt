cmake_minimum_required(VERSION 3.25)
project(sol)

set(CMAKE_CXX_STANDARD 17)

add_executable(sol inputbuf.cc inputbuf.h lexer.h parser.cc)
