cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(sql CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./lib)
set(MYSQL_DIR /shared_files/thirdparty/)

include_directories($ENV{M2EXTERNALS}/include)			
include_directories(${MYSQL_DIR}/include)

add_library(sql STATIC AsyncSQL.h libsql.h Semaphore.h Statement.h stdafx.h Tellwait.h AsyncSQL.cpp Semaphore.cpp Statement.cpp Tellwait.cpp)