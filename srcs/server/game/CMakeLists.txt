##author : yw_massimo | version v.0.0.1

project(game)

cmake_minimum_required(VERSION 3.6.0)

include_directories("../liblua/include")
include_directories(
	/shared_files/thirdparty/include
	${Boost_INCLUDE_DIR}
)

link_directories(
	${extern_LIBRARY}
	${Boost_LIBRARY_DIR}
	/shared_files/thirdparty/library
	${CMAKE_CURRENT_SOURCE_DIR}/../libthecore/lib/
	${CMAKE_CURRENT_SOURCE_DIR}/../libsql
	${CMAKE_CURRENT_SOURCE_DIR}/../liblua/lib
	${CMAKE_CURRENT_SOURCE_DIR}/../libpoly
	${CMAKE_CURRENT_SOURCE_DIR}/../libgame/lib
	/usr/local/lib
)

file(GLOB SOURCES
	src/*.cpp
	src/*.h
)

add_executable(game_yw
	${SOURCES}
)

target_link_libraries(game_yw
	PRIVATE
		thecore
		game
		sql
		poly
		/shared_files/thirdparty/library/libmysqlclient.a
		ssl
		crypto
		lua
		/shared_files/thirdparty/library/libcryptopp.a
		IL
		pthread
		m
		z
		md
		lzo
)
