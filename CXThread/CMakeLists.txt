#CMakeLists.txt
cmake_minimum_required (VERSION 2.6)

#project name
PROJECT(CXThread)

INCLUDE_DIRECTORIES(
../CXCommon/include
../CXLock/include
../CXEvent/include
./include
)


IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
#SET(option WIN32)
 SET(CMAKE_BUILD_TYPE "Debug")
 SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
 SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
 LINK_DIRECTORIES(../../lib/debug)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 LINK_DIRECTORIES(../../lib)
 #SET( CMAKE_CXX_FLAGS "-std=c++11 -O0")
 SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -Wno-unused-variable -pthread")
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")



aux_source_directory(./src DIR_SRCS)
file (GLOB HEADERS "./include/*.h")
SET(DIR_SRCS ${DIR_SRCS} ${HEADERS})
source_group("Header Files" FILES ${HEADERS})

SET(LIBRARY_OUTPUT_PATH ../../lib)


#添加动态库
ADD_LIBRARY(CXThread SHARED ${DIR_SRCS})
#添加静态库
ADD_LIBRARY(CXThread_static STATIC ${DIR_SRCS})


find_package(Threads)
target_link_libraries(CXThread ${CMAKE_THREAD_LIBS_INIT})

#TARGET_LINK_LIBRARIES( CXThread
#pthread
#)


#TARGET_LINK_LIBRARIES( CXThread_static
#pthread
#)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXThread CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXThread_static CXLock_static.lib)
	TARGET_LINK_LIBRARIES(CXThread CXEvent_static.lib)
	TARGET_LINK_LIBRARIES(CXThread_static CXEvent_static.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
    TARGET_LINK_LIBRARIES(CXThread libCXLock.so)
    TARGET_LINK_LIBRARIES(CXThread_static libCXLock_static.a)
	TARGET_LINK_LIBRARIES(CXThread libCXEvent.so)
	TARGET_LINK_LIBRARIES(CXThread_static libCXEvent_static.a)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

#生成动态库的版本号
SET_TARGET_PROPERTIES(CXThread PROPERTIES VERSION 1.0 SOVERSION 1)
#将静态库重新命名为hello
#SET_TARGET_PROPERTIES(CXSpinLock_static PROPERTIES OUTPUT_NAME "hello")

#安装静态库和动态库
#INSTALL(TARGETS CXSpinLock   CXSpinLock_static
 #   LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib)
#安装hello.h
#INSTALL(FILES hello.h DESTINATION include/hello)
