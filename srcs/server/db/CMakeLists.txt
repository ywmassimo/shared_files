##author : yw_massimo | version v.0.0.1

project(db)
cmake_minimum_required(VERSION 3.6.0)

set(extern_INCLUDES ${PROJECT_SOURCE_DIR}/../../../thirdparty/include)
set(extern_LIBRARY  ${PROJECT_SOURCE_DIR}/../../../thirdparty/library)

include_directories(
	${extern_INCLUDES}
	${Boost_INCLUDE_DIR}
)

link_directories(
	${extern_LIBRARY}
	${Boost_LIBRARY_DIR}
	"/usr/local/lib/mysql"
)

file(GLOB SOURCES
	src/*.cpp
	src/*.h
)

add_executable(db_yw
	${SOURCES}
)

target_link_libraries(db_yw LINK_PUBLIC
	thecore
	game
	sql
	poly
	mysqlclient
	ssl
	crypto
	z
	pthread
)
