cmake_minimum_required(VERSION 3.25)
project(proj3_files)

set(CMAKE_CXX_STANDARD 17)

add_executable(proj3_files parser.cc parser.h lexer.cc lexer.h inputbuf.cc inputbuf.h)
