cmake_minimum_required (VERSION 2.6)
project (CXCommunicationClientTestTest)


# The version number.
set (CXCommunicationClientTestTest_VERSION_MAJOR 1)
set (CXCommunicationClientTestTest_VERSION_MINOR 0)



INCLUDE_DIRECTORIES(
../CXCommon/include
../CXEvent/include
../CXLock/include
../CXSocketWrapper/include
../CXThread/include
../CXLog/include
../CXDataParser/include
../CXMemoryCache/include
../CXFile/include
../CXCommunicationClient/include
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
	LINK_DIRECTORIES(../../third_libs/x64/linux)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

SET(EXECUTABLE_OUTPUT_PATH ../../bin)
#add executable file
add_executable(CXCommunicationClientTest ${DIR_SRCS} main.cpp )

TARGET_LINK_LIBRARIES(CXCommunicationClientTest ${LIBRARIES})
IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	TARGET_LINK_LIBRARIES(CXCommunicationClientTest CXCommon_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest CXSocketWrapper_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest CXThread_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest CXLog_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest CXFile_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationClientTest cryptlib.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationClientTest snappy.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationClientTest CXDataParser_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationClientTest CXMemoryCache_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationClientTest CXCommunicationClient_static.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest libCXCommon.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest libCXLock.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest libCXEvent.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest libCXSocketWrapper.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest libCXThread.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest libCXLog.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClientTest libCXFile.so)
	TARGET_LINK_LIBRARIES(CXCommunicationClientTest libCXMemoryCache.so)
	#TARGET_LINK_LIBRARIES(CXCommunicationClientTest libcryptopp.a)
	#TARGET_LINK_LIBRARIES(CXCommunicationClientTest libsnappy.a)
	TARGET_LINK_LIBRARIES(CXCommunicationClientTest uuid)
	TARGET_LINK_LIBRARIES(CXCommunicationClientTest libCXCommunicationClient.so)
	TARGET_LINK_LIBRARIES(CXCommunicationClientTest libCXDataParser.so)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")



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

