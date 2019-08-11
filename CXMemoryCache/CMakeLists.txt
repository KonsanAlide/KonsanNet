#CMakeLists.txt
cmake_minimum_required (VERSION 2.6)

#project name
PROJECT(CXMemoryCache)

INCLUDE_DIRECTORIES(
../CXCommon/include
../CXLock/include
../CXEvent/include
../CXThread/include
 ./include
)

aux_source_directory(src DIR_SRCS)


IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib/debug)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
    SET( CMAKE_CXX_FLAGS "-std=c++11 -O0")
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

ADD_LIBRARY(CXMemoryCache SHARED ${DIR_SRCS})
#添加静态库
ADD_LIBRARY(CXMemoryCache_static STATIC ${DIR_SRCS})

#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ../../lib)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXMemoryCache CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXMemoryCache_static CXLock_static.lib)
	TARGET_LINK_LIBRARIES(CXMemoryCache CXEvent_static.lib)
	TARGET_LINK_LIBRARIES(CXMemoryCache_static CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXMemoryCache CXThread_static.lib)
	TARGET_LINK_LIBRARIES(CXMemoryCache_static CXThread_static.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
    TARGET_LINK_LIBRARIES(CXMemoryCache libCXLock.so)
    TARGET_LINK_LIBRARIES(CXMemoryCache_static libCXLock_static.a)
	TARGET_LINK_LIBRARIES(CXMemoryCache libCXEvent.so)
	TARGET_LINK_LIBRARIES(CXMemoryCache_static libCXEvent_static.a)
    TARGET_LINK_LIBRARIES(CXMemoryCache libCXThread.so)
	TARGET_LINK_LIBRARIES(CXMemoryCache_static libCXThread_static.a)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")



#生成动态库的版本号
SET_TARGET_PROPERTIES(CXMemoryCache PROPERTIES VERSION 1.0 SOVERSION 1)
SET_TARGET_PROPERTIES(CXMemoryCache_static PROPERTIES VERSION 1.0 SOVERSION 1)


