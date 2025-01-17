cmake_minimum_required (VERSION 2.6)
project (CXCommunicationClient)


# The version number.
set (CXCommunicationClient_VERSION_MAJOR 1)
set (CXCommunicationClient_VERSION_MINOR 0)



INCLUDE_DIRECTORIES(
../CXCommon/include
../CXEvent/include
../CXLock/include
../CXSocketWrapper/include
../CXThread/include
../CXFile/include
../CXLog/include
../CXDataParser/include
../CXMemoryCache/include
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


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib/debug)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


SET(LIBRARY_OUTPUT_PATH ../../lib)

ADD_LIBRARY(CXCommunicationClient SHARED ${DIR_COMMON_SRCS} ${DIR_SRCS})
#添加静态库
ADD_LIBRARY(CXCommunicationClient_static STATIC ${DIR_COMMON_SRCS} ${DIR_SRCS})

#add executable file
#add_executable(CXCommunicationClient ${DIR_SRCS} main.cpp )

TARGET_LINK_LIBRARIES(CXCommunicationClient ${LIBRARIES})
IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	TARGET_LINK_LIBRARIES(CXCommunicationClient CXCommon_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient CXSocketWrapper_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient CXThread_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient CXLog_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient CXFile_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationClient CXDataParser_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationClient CXMemoryCache_static.lib)
	

	TARGET_LINK_LIBRARIES(CXCommunicationClient_static CXCommon_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static CXSocketWrapper_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static CXThread_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static CXLog_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static CXFile_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationClient_static CXDataParser_static.lib)
	TARGET_LINK_LIBRARIES(CXCommunicationClient_static CXMemoryCache_static.lib)
	
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	TARGET_LINK_LIBRARIES(CXCommunicationClient libCXCommon.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClient libCXLock.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClient libCXEvent.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClient libCXSocketWrapper.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClient libCXThread.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClient libCXLog.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClient libCXFile.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClient uuid)
	TARGET_LINK_LIBRARIES(CXCommunicationClient libCXDataParser.so)
	TARGET_LINK_LIBRARIES(CXCommunicationClient libCXMemoryCache.so)

	TARGET_LINK_LIBRARIES(CXCommunicationClient_static libCXCommon_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static libCXLock_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static libCXEvent_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static libCXSocketWrapper_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static libCXThread_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static libCXLog_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static libCXFile_static.a)
    TARGET_LINK_LIBRARIES(CXCommunicationClient_static uuid)
	TARGET_LINK_LIBRARIES(CXCommunicationClient_static libCXDataParser_static.a)
	TARGET_LINK_LIBRARIES(CXCommunicationClient_static libCXMemoryCache_static.a)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")



#TARGET_LINK_LIBRARIES（CXCommunicationClient debug 3PDebugLib）
#TARGET_LINK_LIBRARIES（CXCommunicationClient optimized 3PReleaseLib）

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

