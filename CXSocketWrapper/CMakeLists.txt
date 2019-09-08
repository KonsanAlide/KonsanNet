#CMakeLists.txt
cmake_minimum_required (VERSION 2.6)

#project name  
PROJECT(CXSocketWrapper)  

INCLUDE_DIRECTORIES( ./include )


INCLUDE_DIRECTORIES( 
../CXCommon/include 
./include
)
INCLUDE_DIRECTORIES(  )

aux_source_directory(./src DIR_SRCS)
file (GLOB HEADERS "./include/*.h")
SET(DIR_SRCS ${DIR_SRCS} ${HEADERS})
source_group("Header Files" FILES ${HEADERS})

SET(LIBRARY_OUTPUT_PATH ../../lib)

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
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")



#添加动态库  
ADD_LIBRARY(CXSocketWrapper SHARED ${DIR_SRCS})  
#添加静态库  
ADD_LIBRARY(CXSocketWrapper_static STATIC ${DIR_SRCS}) 

TARGET_LINK_LIBRARIES(CXSocketWrapper ${LIBRARIES})
TARGET_LINK_LIBRARIES(CXSocketWrapper_static ${LIBRARIES})


  
#生成动态库的版本号  
SET_TARGET_PROPERTIES(CXSocketWrapper PROPERTIES VERSION 1.0 SOVERSION 1) 
SET_TARGET_PROPERTIES(CXSocketWrapper_static PROPERTIES VERSION 1.0 SOVERSION 1) 
   
#将静态库重新命名为hello  
#SET_TARGET_PROPERTIES(CXSpinLock_static PROPERTIES OUTPUT_NAME "hello")  
  
#安装静态库和动态库  
#INSTALL(TARGETS CXSpinLock   CXSpinLock_static   
 #   LIBRARY DESTINATION lib   
#    ARCHIVE DESTINATION lib)  
#安装hello.h  
#INSTALL(FILES hello.h DESTINATION include/hello) 