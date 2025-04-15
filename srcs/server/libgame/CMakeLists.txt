cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(game CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./lib)

include_directories("./include")
include_directories("../../thirdparty/include")

add_library(game STATIC src/attribute.cc src/grid.cc src/targa.cc)