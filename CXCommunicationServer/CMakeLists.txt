cmake_minimum_required (VERSION 2.6)
project (CXCommunicationServer)


# The version number.
set (CXCommunicationServer_VERSION_MAJOR 1)
set (CXCommunicationServer_VERSION_MINOR 0)



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
../CXDataParser/include
../CXCommunicationClient/include
../CXTask/include
./include
)

aux_source_directory(./src DIR_SRCS)
file (GLOB HEADERS "./include/*.h")
SET(DIR_SRCS ${DIR_SRCS} ${HEADERS})
source_group("Header Files" FILES ${HEADERS})

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 #SET(option WIN32)
 SET(CMAKE_BUILD_TYPE "Debug")
 SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
 SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
 #set extern libraries
 SET(LIBRARIES
     ws2_32.lib
 )
 message("WINDOWS SYSTEM")
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
SET( CMAKE_CXX_FLAGS "-std=c++11 -O0")
SET(CMAKE_BUILD_TYPE "Debug")
SET(LIBRARIES
     libuuid_static.a
 )
 #SET(defs -DUNICODE -D_UNICODE)
 #INCLUDE_DIRECTORIES( ./include/linux )
 SET(LIBRARIES

 )
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

SET(LIBRARY_OUTPUT_PATH ../../lib)
#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib/debug)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


SET(LIBRARY_OUTPUT_PATH ../../lib)

#add executable file
#add_executable(CXCommunicationServer ${DIR_SRCS} main.cpp )
ADD_LIBRARY(CXCommunicationServer SHARED ${DIR_COMMON_SRCS} ${DIR_SRCS})
#添加静态库
ADD_LIBRARY(CXCommunicationServer_static STATIC ${DIR_COMMON_SRCS} ${DIR_SRCS})

TARGET_LINK_LIBRARIES(CXCommunicationServer ${LIBRARIES})

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	TARGET_LINK_LIBRARIES(CXCommunicationServer CXCommon_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer CXSocketWrapper_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer CXThread_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer CXMemoryCache_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer CXLog_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer CXFile_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServer CXContainer_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServer CXDataParser_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServer CXCommunicationClient_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServer CXTask_static.lib)
	
	TARGET_LINK_LIBRARIES(CXCommunicationServer_static CXCommon_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static CXSocketWrapper_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static CXThread_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static CXMemoryCache_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static CXLog_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static CXFile_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServer_static CXContainer_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServer_static CXDataParser_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServer_static CXCommunicationClient_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationServer_static CXTask_static.lib)

ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	TARGET_LINK_LIBRARIES(CXCommunicationServer libCXCommon.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libCXLock.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libCXEvent.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libCXSocketWrapper.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libCXThread.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libCXMemoryCache.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libCXLog.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libCXFile.so)
	TARGET_LINK_LIBRARIES(CXCommunicationServer libCXContainer.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer uuid)
	TARGET_LINK_LIBRARIES(CXCommunicationServer libCXDataParser.so)
	TARGET_LINK_LIBRARIES(CXCommunicationServer libCXCommunicationClient.so)
	TARGET_LINK_LIBRARIES(CXCommunicationServer libCXTask.so)

	TARGET_LINK_LIBRARIES(CXCommunicationServer_static libCXCommon_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static libCXLock_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static libCXEvent_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static libCXSocketWrapper_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static libCXThread_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static libCXMemoryCache_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static libCXLog_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static libCXFile_static.a)
	TARGET_LINK_LIBRARIES(CXCommunicationServer_static libCXContainer_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationServer_static uuid)
	TARGET_LINK_LIBRARIES(CXCommunicationServer_static libCXDataParser_static.a)
	TARGET_LINK_LIBRARIES(CXCommunicationServer_static libCXCommunicationClient_static.a)
	TARGET_LINK_LIBRARIES(CXCommunicationServer_static libCXTask_static.a)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


#TARGET_LINK_LIBRARIES（CXCommunicationServer debug 3PDebugLib）
#TARGET_LINK_LIBRARIES（CXCommunicationServer optimized 3PReleaseLib）

#add executable file
#ADD_EXECUTABLE(../bin/bin ${TEST_MATH})
#add link library
#TARGET_LINK_LIBRARIES(../bin/bin ${LIBRARIES}

# add the MathFunctions library?
# if (USE_MYMATH)
#   include_directories ("${PROJECT_SOURCE_DIR}/MathFunctions")
#   add_subdirectory (MathFunctions)
#   set (EXTRA_LIBS ${EXTRA_LIBS} MathFunctions)
# endif (USE_MYMATH)

# add the executable

#target_link_libraries (Tutorial  ${EXTRA_LIBS})

# add the install targets
#install (TARGETS Tutorial DESTINATION bin)
#install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h"
#         DESTINATION include)

