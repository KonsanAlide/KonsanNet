#CMakeLists.txt
cmake_minimum_required (VERSION 2.6)

#project name
PROJECT(CXProxyServerTest)

INCLUDE_DIRECTORIES(
../CXCommon/include
../CXEvent/include
../CXLock/include
../CXSocketWrapper/include
../CXMemoryCache/include
../CXThread/include
../CXLog/include
../CXFile/include
../CXContainer/include
../CXCommunicationServer/include
../CXDataParser/include
../CXTask/include
./include
)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 #SET(option WIN32)
 SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
 SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
 #set extern libraries
 SET(LIBRARIES
     ws2_32.lib
 )
 message("WINDOWS SYSTEM")
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
SET(CMAKE_CXX_FLAGS "-std=c++11 -O0")
SET(CMAKE_BUILD_TYPE "Debug")
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

aux_source_directory(./src DIR_SRCS)
file (GLOB HEADERS "./include/*.h")
SET(DIR_SRCS ${DIR_SRCS} ${HEADERS})
source_group("Header Files" FILES ${HEADERS})


#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	LINK_DIRECTORIES(../../lib/debug)
	LINK_DIRECTORIES(../../third_libs/i386/windows/debug)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
	LINK_DIRECTORIES(../../third_libs/x64/linux)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

SET(EXECUTABLE_OUTPUT_PATH ../../bin)
add_executable(CXProxyServerTest ${DIR_SRCS} main.cpp )

TARGET_LINK_LIBRARIES(CXProxyServerTest ${LIBRARIES})

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	TARGET_LINK_LIBRARIES(CXProxyServerTest CXCommon_static.lib)
    TARGET_LINK_LIBRARIES(CXProxyServerTest CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXProxyServerTest CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXProxyServerTest CXSocketWrapper_static.lib)
    TARGET_LINK_LIBRARIES(CXProxyServerTest CXThread_static.lib)
    TARGET_LINK_LIBRARIES(CXProxyServerTest CXMemoryCache_static.lib)
    TARGET_LINK_LIBRARIES(CXProxyServerTest CXLog_static.lib)
    TARGET_LINK_LIBRARIES(CXProxyServerTest CXFile_static.lib)
	TARGET_LINK_LIBRARIES(CXProxyServerTest CXContainer_static.lib)
	TARGET_LINK_LIBRARIES(CXProxyServerTest CXDataParser_static.lib)
	TARGET_LINK_LIBRARIES(CXProxyServerTest CXCommunicationServer_static.lib)
	TARGET_LINK_LIBRARIES(CXProxyServerTest cryptlib.lib)
	TARGET_LINK_LIBRARIES(CXProxyServerTest snappy.lib)
	TARGET_LINK_LIBRARIES(CXProxyServerTest CXTask_static.lib)
	TARGET_LINK_LIBRARIES(CXProxyServerTest CXCommunicationClient_static.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXProxyServerTest libCXCommon.so)
    TARGET_LINK_LIBRARIES(CXProxyServerTest libCXLock.so)
    TARGET_LINK_LIBRARIES(CXProxyServerTest libCXEvent.so)
    TARGET_LINK_LIBRARIES(CXProxyServerTest libCXSocketWrapper.so)
    TARGET_LINK_LIBRARIES(CXProxyServerTest libCXThread.so)
    TARGET_LINK_LIBRARIES(CXProxyServerTest libCXMemoryCache.so)
    TARGET_LINK_LIBRARIES(CXProxyServerTest libCXLog.so)
    TARGET_LINK_LIBRARIES(CXProxyServerTest libCXFile.so)
	TARGET_LINK_LIBRARIES(CXProxyServerTest libCXContainer.so)
    TARGET_LINK_LIBRARIES(CXProxyServerTest uuid)
	TARGET_LINK_LIBRARIES(CXProxyServerTest libCXCommunicationServer.so)
	TARGET_LINK_LIBRARIES(CXProxyServerTest libCXDataParser.so)
	TARGET_LINK_LIBRARIES(CXProxyServerTest libCXTask.so)
	TARGET_LINK_LIBRARIES(CXProxyServerTest libCXCommunicationClient.so)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")



