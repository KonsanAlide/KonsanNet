#CMakeLists.txt
cmake_minimum_required (VERSION 2.6)

#project name
PROJECT(CXDataParser)

INCLUDE_DIRECTORIES(
../CXCommon/include
../third_libs_source/cryptopp820
#../third_libs_source/snappy-1.1.7
./include
)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 #SET(option WIN32)
 SET(CMAKE_BUILD_TYPE "Debug")
 SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
 SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
 #set extern libraries
 SET(LIBRARIES
     #ws2_32.lib
 )
 message("WINDOWS SYSTEM")
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 #SET(defs -DUNICODE -D_UNICODE)
 #INCLUDE_DIRECTORIES( ./include/linux )
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")



aux_source_directory(./src DIR_SRCS)
file (GLOB HEADERS "./include/*.h")
SET(DIR_SRCS ${DIR_SRCS} ${HEADERS})
source_group("Header Files" FILES ${HEADERS})

SET(LIBRARY_OUTPUT_PATH ../../lib)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	LINK_DIRECTORIES(../../lib/debug)
	LINK_DIRECTORIES(../../third_libs/i386/windows/debug)
	INCLUDE_DIRECTORIES(../third_libs_source/snappy-1.1.7/include/windows)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
	LINK_DIRECTORIES(../../third_libs/x64/linux)
	INCLUDE_DIRECTORIES(../third_libs_source/snappy-1.1.7/include/linux)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


#添加动态库
ADD_LIBRARY(CXDataParser SHARED ${DIR_SRCS})
#添加静态库
ADD_LIBRARY(CXDataParser_static STATIC ${DIR_SRCS})

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXDataParser CXCommon_static.lib)
	TARGET_LINK_LIBRARIES(CXDataParser cryptlib.lib)
	TARGET_LINK_LIBRARIES(CXDataParser snappy.lib)
	
	TARGET_LINK_LIBRARIES(CXDataParser_static CXCommon_static.lib)
	TARGET_LINK_LIBRARIES(CXDataParser_static cryptlib.lib)
	TARGET_LINK_LIBRARIES(CXDataParser_static snappy.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXDataParser libCXCommon_static.a)
    TARGET_LINK_LIBRARIES(CXDataParser libcryptopp.a)
	TARGET_LINK_LIBRARIES(CXDataParser libsnappy.a)

	TARGET_LINK_LIBRARIES(CXDataParser_static libCXCommon_static.a)
	TARGET_LINK_LIBRARIES(CXDataParser_static libcryptopp.a)
	TARGET_LINK_LIBRARIES(CXDataParser_static libsnappy.a)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


#生成动态库的版本号
SET_TARGET_PROPERTIES(CXDataParser PROPERTIES VERSION 1.0 SOVERSION 1)
#将静态库重新命名为hello
#SET_TARGET_PROPERTIES(CXSpinLock_static PROPERTIES OUTPUT_NAME "hello")

#安装静态库和动态库
#INSTALL(TARGETS CXSpinLock   CXSpinLock_static
 #   LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib)
#安装hello.h
#INSTALL(FILES hello.h DESTINATION include/hello)
