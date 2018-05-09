#CMakeLists.txt
cmake_minimum_required (VERSION 2.6)

#project name  
PROJECT(CXMemoryCache)  

INCLUDE_DIRECTORIES(
../CXCommon/include
../CXLock/include
 ./include 
)

aux_source_directory(src DIR_SRCS)

LINK_DIRECTORIES(../../lib/Release)

#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ../../lib)

ADD_LIBRARY(CXMemoryCache SHARED ${DIR_SRCS})  
#添加静态库  
ADD_LIBRARY(CXMemoryCache_static STATIC ${DIR_SRCS})  
TARGET_LINK_LIBRARIES(CXMemoryCache CXLock_static.lib)
TARGET_LINK_LIBRARIES(CXMemoryCache_static CXLock_static.lib)
  
#生成动态库的版本号  
SET_TARGET_PROPERTIES(CXMemoryCache PROPERTIES VERSION 1.0 SOVERSION 1)  
SET_TARGET_PROPERTIES(CXMemoryCache_static PROPERTIES VERSION 1.0 SOVERSION 1)  


