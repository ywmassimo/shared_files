cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(thecore CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./lib)

include_directories("../../thirdparty/include")	
include_directories("./include/")

add_library(thecore STATIC src/main.c include/stdafx.h src/buffer.c src/des.c src/fdwatch.c src/gost.c src/hangul.c src/heart.c src/kstbl.c src/log.c src/signal.c src/socket.c src/tea.c src/utils.cpp src/xdirent.c src/xgetopt.c src/xmd5.c include/buffer.h include/crypt.h include/fdwatch.h include/hangul.h include/heart.h include/kstbl.h include/log.h include/main.h include/signal.h include/socket.h include/typedef.h include/utils.h include/xdirent.h include/xgetopt.h include/xmd5.h include/base32.h src/base32.c  include/hmac.h  src/hmac.c src/sha1.c include/sha1.h src/topt.c include/topt.h)

SET_SOURCE_FILES_PROPERTIES( src/buffer.c src/des.c src/DES_table.h src/fdwatch.c src/gost.c src/hangul.c src/heart.c src/kstbl.c src/log.c src/main.c src/signal.c src/socket.c src/tea.c src/tea.s src/utils.c src/xdirent.c src/xgetopt.c src/xmd5.c PROPERTIES LANGUAGE CXX )
SET_TARGET_PROPERTIES(thecore PROPERTIES LINKER_LANGUAGE CXX)
