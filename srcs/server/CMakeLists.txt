##author : yw_massimo | version v.0.0.1

cmake_minimum_required(VERSION 3.6.0)
#
SET(CMAKE_COLOR_MAKEFILE ON)
#
project(all)
#
set(OUTPUT_DIR  ${PROJECT_SOURCE_DIR}/bin) # Relative to the main src directory! Folders will be created
#
#set(COMPILER_SUBDIR "clang")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

################################################################################
# Global compiler options
################################################################################
find_package(LLVM 19.1.7 REQUIRED)
#find_library(lzo2 REQUIRED)
#find_package(CryptoPP REQUIRED)
#include(CMakeDetermineCXXCompiler)
set(CMAKE_CXX_COMPILER_ID "Clang")
set(CMAKE_CXX_COMPILER_TARGET "i386-portbld-freebsd13.1")
set(CMAKE_C_COMPILER_TARGET "i386-portbld-freebsd13.1")
set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ Standard (toolchain)" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED YES CACHE BOOL "C++ Standard required" FORCE)
set(CMAKE_CXX_EXTENSIONS NO CACHE BOOL "C++ Standard extensions" FORCE)

# Common flags for all compiler configurations 
#set(SANITIZER_OPT "-fsanitize=address -fno-omit-frame-pointer  -fsanitize-recover=all")
#set(COMMON_C_CXX_FLAGS "-ggdb -g -o -O2 -Waddress-of-temporary -pthread -pipe -fstack-protector-all -fexceptions -D__MARIADB__ -Wno-deprecated-register -D_THREAD_SAFE")

set(COMMON_C_CXX_FLAGS "-g -o -O2 -static -pthread -pipe -fstack-protector-all -fexceptions -D__MARIADB__ -Wno-deprecated-register -D_THREAD_SAFE")

#set(CMAKE_C_FLAGS "${COMMON_C_CXX_FLAGS} ${SANITIZER_OPT}")
#set(CMAKE_CXX_FLAGS "${COMMON_C_CXX_FLAGS} ${SANITIZER_OPT} -D__CLANG__ -stdlib=libc++ -Wno-invalid-source-encoding -Wno-unused-private-field")
set(CMAKE_C_FLAGS "${COMMON_C_CXX_FLAGS} ")
set(CMAKE_CXX_FLAGS "${COMMON_C_CXX_FLAGS} -D__CLANG__ -stdlib=libc++ -Wno-invalid-source-encoding -Wno-unused-private-field")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_DISTRIBUTE "${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DISTRIBUTE "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS}")

################################################################################
# Global linker options
################################################################################

# Common flags for all linker configurations 
set(CMAKE_EXE_LINKER_FLAGS "")
set(CMAKE_MODULE_LINKER_FLAGS "")
set(CMAKE_SHARED_LINKER_FLAGS "")
set(CMAKE_STATIC_LINKER_FLAGS "")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_DISTRIBUTE "${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS_DISTRIBUTE "${CMAKE_MODULE_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_DISTRIBUTE "${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_STATIC_LINKER_FLAGS_DISTRIBUTE "${CMAKE_STATIC_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")

include_directories(${PROJECT_SOURCE_DIR}/../thirdparty/include)

add_subdirectory(libthecore)
add_subdirectory(libsql)
add_subdirectory(libpoly)
add_subdirectory(libgame)
add_subdirectory(liblua)
add_subdirectory(db)
add_subdirectory(game)
