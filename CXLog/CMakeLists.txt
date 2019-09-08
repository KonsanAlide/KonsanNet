#CMakeLists.txt
cmake_minimum_required (VERSION 2.6)

#project name
PROJECT(CXLog)

INCLUDE_DIRECTORIES(
../CXCommon/include
../CXEvent/include
../CXThread/include
../CXLock/include
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
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


#添加动态库
ADD_LIBRARY(CXLog SHARED ${DIR_SRCS})
#添加静态库
ADD_LIBRARY(CXLog_static STATIC ${DIR_SRCS})

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXLog CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXLog CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXLog CXThread_static.lib)

    TARGET_LINK_LIBRARIES(CXLog_static CXLock_static.lib)
    TARGET_LINK_LIBRARIES(CXLog_static CXEvent_static.lib)
    TARGET_LINK_LIBRARIES(CXLog_static CXThread_static.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

    #TARGET_LINK_LIBRARIES(CXLog libCXLock_static.so)
    #TARGET_LINK_LIBRARIES(CXLog libCXEvent_static.so)
    #TARGET_LINK_LIBRARIES(CXLog libCXThread_static.so)

    TARGET_LINK_LIBRARIES(CXLog libCXLock.so)
    TARGET_LINK_LIBRARIES(CXLog libCXEvent.so)
    TARGET_LINK_LIBRARIES(CXLog libCXThread.so)

    #TARGET_LINK_LIBRARIES(CXLog_static libCXLock_static.a)
    #TARGET_LINK_LIBRARIES(CXLog_static libCXEvent_static.a)
    #TARGET_LINK_LIBRARIES(CXLog_static libCXThread_static.a)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


#生成动态库的版本号
SET_TARGET_PROPERTIES(CXLog PROPERTIES VERSION 1.0 SOVERSION 1)
#将静态库重新命名为hello
#SET_TARGET_PROPERTIES(CXSpinLock_static PROPERTIES OUTPUT_NAME "hello")

#安装静态库和动态库
#INSTALL(TARGETS CXSpinLock   CXSpinLock_static
 #   LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib)
#安装hello.h
#INSTALL(FILES hello.h DESTINATION include/hello)
