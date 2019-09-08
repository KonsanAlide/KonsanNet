cmake_minimum_required (VERSION 2.6)
project (CXTestSample)


# The version number.
set (CXTestSample_VERSION_MAJOR 1)
set (CXTestSample_VERSION_MINOR 0)


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
../CXCommunicationClient/include
../CXDataParser/include
../third_libs_source/snappy-1.1.7
../third_libs_source/cryptopp820
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
 #SET(defs -DUNICODE -D_UNICODE)
 #INCLUDE_DIRECTORIES( ./include/linux )
 SET(CMAKE_BUILD_TYPE "Debug")
 SET(LIBRARIES

 )
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	LINK_DIRECTORIES(../../lib/debug)
	LINK_DIRECTORIES(../../third_libs/i386/windows/debug)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
	LINK_DIRECTORIES(../../third_libs/i386/linux)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

SET(EXECUTABLE_OUTPUT_PATH ../../bin)

#add executable file
add_executable(CXTestSample ${DIR_SRCS} main.cpp )


IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	TARGET_LINK_LIBRARIES(CXTestSample CXCommon_static.lib)
    TARGET_LINK_LIBRARIES(CXTestSample CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXTestSample CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXTestSample CXSocketWrapper_static.lib)
    TARGET_LINK_LIBRARIES(CXTestSample CXThread_static.lib)
    TARGET_LINK_LIBRARIES(CXTestSample CXMemoryCache_static.lib)
    TARGET_LINK_LIBRARIES(CXTestSample CXLog_static.lib)
    TARGET_LINK_LIBRARIES(CXTestSample CXFile_static.lib)
	TARGET_LINK_LIBRARIES(CXTestSample CXContainer_static.lib)
	TARGET_LINK_LIBRARIES(CXTestSample CXDataParser_static.lib)
	TARGET_LINK_LIBRARIES(CXTestSample CXCommunicationServer_static.lib)
	TARGET_LINK_LIBRARIES(CXTestSample cryptlib.lib)
	TARGET_LINK_LIBRARIES(CXTestSample snappy.lib)
	TARGET_LINK_LIBRARIES(CXTestSample CXTask_static.lib)
	TARGET_LINK_LIBRARIES(CXTestSample CXCommunicationClient_static.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXTestSample libCXCommon.so)
    TARGET_LINK_LIBRARIES(CXTestSample libCXLock.so)
    TARGET_LINK_LIBRARIES(CXTestSample libCXEvent.so)
    TARGET_LINK_LIBRARIES(CXTestSample libCXSocketWrapper.so)
    TARGET_LINK_LIBRARIES(CXTestSample libCXThread.so)
    TARGET_LINK_LIBRARIES(CXTestSample libCXMemoryCache.so)
    TARGET_LINK_LIBRARIES(CXTestSample libCXFile.so)
    TARGET_LINK_LIBRARIES(CXTestSample libCXLog.so)
	TARGET_LINK_LIBRARIES(CXTestSample libCXContainer.so)
    TARGET_LINK_LIBRARIES(CXTestSample uuid)
    TARGET_LINK_LIBRARIES(CXTestSample libCXDataParser.so)
	TARGET_LINK_LIBRARIES(CXTestSample libCXTask.so)
    TARGET_LINK_LIBRARIES(CXTestSample libCXCommunicationClient.so)
	TARGET_LINK_LIBRARIES(CXTestSample libCXCommunicationServer.so)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

TARGET_LINK_LIBRARIES(CXTestSample ${LIBRARIES})


#TARGET_LINK_LIBRARIES（CXCommunicationClientTest debug 3PDebugLib）
#TARGET_LINK_LIBRARIES（CXCommunicationClientTest optimized 3PReleaseLib）

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

