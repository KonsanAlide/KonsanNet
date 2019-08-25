cmake_minimum_required (VERSION 2.6)
project (CXTestSample)


# The version number.
set (CXTestSample_VERSION_MAJOR 1)
set (CXTestSample_VERSION_MINOR 0)



INCLUDE_DIRECTORIES(
../CXCommon/include
../third_libs_source/snappy-1.1.7
../third_libs_source/cryptopp820
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

TARGET_LINK_LIBRARIES(CXTestSample ${LIBRARIES})
IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	TARGET_LINK_LIBRARIES(CXTestSample CXCommon_static.lib)
	TARGET_LINK_LIBRARIES(CXTestSample cryptlib.lib)
	TARGET_LINK_LIBRARIES(CXTestSample snappy.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXTestSample libCXCommon.so)
	TARGET_LINK_LIBRARIES(CXTestSample cryptlib.so)
	TARGET_LINK_LIBRARIES(CXTestSample snappy.so)
    
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

