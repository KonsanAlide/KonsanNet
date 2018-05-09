#CMakeLists.txt
cmake_minimum_required (VERSION 2.6)

#project name  
PROJECT(CXEvent)  

INCLUDE_DIRECTORIES( 
../CXCommon/include 
./include
)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 SET(option WIN32)
 #set extern libraries  
 SET(LIBRARIES  
      
 )  
 message("WINDOWS SYSTEM")
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 #SET(defs -DUNICODE -D_UNICODE)
 #INCLUDE_DIRECTORIES( ./include/linux )
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")



aux_source_directory(./src DIR_SRCS)

SET(LIBRARY_OUTPUT_PATH ../../lib)


#添加动态库  
ADD_LIBRARY(CXEvent SHARED ${DIR_SRCS})  
#添加静态库  
ADD_LIBRARY(CXEvent_static STATIC ${DIR_SRCS})  
  
#生成动态库的版本号  
SET_TARGET_PROPERTIES(CXEvent PROPERTIES VERSION 1.0 SOVERSION 1)
SET_TARGET_PROPERTIES(CXEvent_static PROPERTIES VERSION 1.0 SOVERSION 1)   
#将静态库重新命名为hello  
#SET_TARGET_PROPERTIES(CXSpinLock_static PROPERTIES OUTPUT_NAME "hello")  
  
#安装静态库和动态库  
#INSTALL(TARGETS CXSpinLock   CXSpinLock_static   
 #   LIBRARY DESTINATION lib   
#    ARCHIVE DESTINATION lib)  
#安装hello.h  
#INSTALL(FILES hello.h DESTINATION include/hello) 