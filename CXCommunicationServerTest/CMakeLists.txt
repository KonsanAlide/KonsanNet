#CMakeLists.txt
cmake_minimum_required (VERSION 2.6)

#project name
PROJECT(CXCommunicationServerTest)

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
./include
)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 SET(option WIN32)
 #set extern libraries
 SET(LIBRARIES
     ws2_32.lib
 )
 message("WINDOWS SYSTEM")
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
SET(CMAKE_CXX_FLAGS "-std=c++11 -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
SET(CMAKE_BUILD_TYPE "Debug")
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

aux_source_directory(src DIR_SRCS)


#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	LINK_DIRECTORIES(../../lib/debug)
	LINK_DIRECTORIES(../../third_libs/i386/windows/debug)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
	LINK_DIRECTORIES(../../third_libs/i386/linux)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

SET(EXECUTABLE_OUTPUT_PATH ../../bin)
add_executable(CXCommunicationServerTest ${DIR_SRCS} main.cpp )

TARGET_LINK_LIBRARIES(CXCommunicationServerTest ${LIBRARIES})

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	TARGET_LINK_LIBRARIES(CXCommunicationServerTest CXCommon_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest CXSocketWrapper_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest CXThread_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest CXMemoryCache_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest CXLog_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest CXFile_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServerTest CXContainer_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServerTest CXDataParser_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServerTest CXCommunicationServer_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServerTest cryptlib.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServerTest snappy.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest libCXCommon.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest libCXLock.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest libCXEvent.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest libCXSocketWrapper.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest libCXThread.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest libCXMemoryCache.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest libCXLog.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest libCXFile.so)
	TARGET_LINK_LIBRARIES(CXCommunicationServerTest libCXContainer.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServerTest uuid)
	TARGET_LINK_LIBRARIES(CXCommunicationServerTest libCXCommunicationServer.so)
	TARGET_LINK_LIBRARIES(CXCommunicationServerTest libCXDataParser.so)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")



