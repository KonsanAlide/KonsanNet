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
SET( CMAKE_CXX_FLAGS "-std=c++11 -O3")
SET(LIBRARIES
     libuuid.so
 )
 #SET(defs -DUNICODE -D_UNICODE)
 #INCLUDE_DIRECTORIES( ./include/linux )
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
add_executable(CXCommunicationServer ${DIR_SRCS} main.cpp )

TARGET_LINK_LIBRARIES(CXCommunicationServer ${LIBRARIES})

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXCommunicationServer CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer CXSocketWrapper_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer CXThread_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer CXMemoryCache_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer cxlog_static.lib)
    TARGET_LINK_LIBRARIES(CXCommunicationServer cxfile_static.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXCommunicationServer libCXLock.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libCXEvent.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libCXSocketWrapper.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libCXThread.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libCXMemoryCache.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libcxlog.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer libcxfile.so)
    TARGET_LINK_LIBRARIES(CXCommunicationServer uuid)
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

