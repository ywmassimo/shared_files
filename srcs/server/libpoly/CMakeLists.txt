cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(poly CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./lib)

include_directories("/shared_files/thirdparty/include")	
include_directories("/usr/include")

add_library(poly STATIC Base.h Base.cc Poly.h Poly.cc SymTable.h SymTable.cc Symbol.h Symbol.cc Constants.h)