cmake_minimum_required(VERSION 2.8)
project(auth_test)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I../include")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
add_executable(auth_test auth_test.c)

#include(../common.mk)
#findPackage(libnetapi REQUIRED
#	HINTS ${libnetapiPrefix}
#	HEADERS net_common.h
#	INC_PATH_SUFFIX include/libnetapi
#	SHARED_LIBS libnetapi
#)

#target_include_directories(netapi_test PRIVATE ${libnetapi_INCLUDE_DIRS})

target_link_libraries(auth_test libauth.a)

install(TARGETS auth_test
	RUNTIME DESTINATION bin
)
