#CMakeLists.txt
cmake_minimum_required (VERSION 2.6)

#project name
PROJECT(CXDataParser)

INCLUDE_DIRECTORIES(
../CXCommon/include
../third_libs_source/cryptopp820
../third_libs_source/snappy-1.1.7
./include
)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 SET(option WIN32)
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

SET(LIBRARY_OUTPUT_PATH ../../lib)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	LINK_DIRECTORIES(../../lib/debug)
	LINK_DIRECTORIES(../../third_libs/i386/windows/debug)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
	LINK_DIRECTORIES(../../third_libs/i386/linux)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


#添加动态库
ADD_LIBRARY(CXDataParser SHARED ${DIR_SRCS})
#添加静态库
ADD_LIBRARY(CXDataParser_static STATIC ${DIR_SRCS})

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	TARGET_LINK_LIBRARIES(CXDataParser cryptlib.lib)
	TARGET_LINK_LIBRARIES(CXDataParser snappy.lib)
	TARGET_LINK_LIBRARIES(CXDataParser_static cryptlib.lib)
	TARGET_LINK_LIBRARIES(CXDataParser_static snappy.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

    #TARGET_LINK_LIBRARIES(CXDataParser libCXLock_static.so)
    #TARGET_LINK_LIBRARIES(CXDataParser libCXEvent_static.so)
    #TARGET_LINK_LIBRARIES(CXDataParser libCXThread_static.so)

	TARGET_LINK_LIBRARIES(CXDataParser cryptlib.so)
	TARGET_LINK_LIBRARIES(CXDataParser snappy.so)

    #TARGET_LINK_LIBRARIES(CXDataParser_static libCXLock_static.a)
    #TARGET_LINK_LIBRARIES(CXDataParser_static libCXEvent_static.a)
    #TARGET_LINK_LIBRARIES(CXDataParser_static libCXThread_static.a)
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
