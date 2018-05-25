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
../CXLog/include
./include
)

aux_source_directory(src DIR_SRCS)


IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 SET(option WIN32)
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

#add executable file
add_executable(CXCommunicationClient ${DIR_SRCS} main.cpp )

TARGET_LINK_LIBRARIES(CXCommunicationClient ${LIBRARIES})
IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXCommunicationClient CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient CXSocketWrapper_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient CXThread_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient cxlog_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationClient cxfile_static.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXCommunicationClient libCXLock.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClient libCXEvent.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClient libCXSocketWrapper.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClient libCXThread.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClient libcxlog.so)
    TARGET_LINK_LIBRARIES(CXCommunicationClient libcxfile.so)
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

