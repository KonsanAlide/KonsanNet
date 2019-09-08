#CMakeLists.txt
cmake_minimum_required (VERSION 2.6)

#project name  
PROJECT(CXLock)  

INCLUDE_DIRECTORIES( ./include )
IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 #SET(option WIN32)
 SET(CMAKE_BUILD_TYPE "Debug")
 SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
 SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
 #SET(win32_LIBRARIES comctl32.lib shlwapi.lib shell32.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib )
 INCLUDE_DIRECTORIES( ./include/windows )
 message("WINDOWS SYSTEM")
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 #SET(defs -DUNICODE -D_UNICODE)
 INCLUDE_DIRECTORIES( ./include/linux )
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")



aux_source_directory(./src DIR_SRCS)
file (GLOB HEADERS "./include/*.h")
SET(DIR_SRCS ${DIR_SRCS} ${HEADERS})
source_group("Header Files" FILES ${HEADERS})

SET(LIBRARY_OUTPUT_PATH ../../lib)


#添加动态库  
ADD_LIBRARY(CXLock SHARED ${DIR_SRCS})  
#添加静态库  
ADD_LIBRARY(CXLock_static STATIC ${DIR_SRCS})  
  
#生成动态库的版本号  
SET_TARGET_PROPERTIES(CXLock PROPERTIES VERSION 1.0 SOVERSION 1)  
#将静态库重新命名为hello  
#SET_TARGET_PROPERTIES(CXSpinLock_static PROPERTIES OUTPUT_NAME "hello")  
  
#安装静态库和动态库  
#INSTALL(TARGETS CXSpinLock   CXSpinLock_static   
 #   LIBRARY DESTINATION lib   
#    ARCHIVE DESTINATION lib)  
#安装hello.h  
#INSTALL(FILES hello.h DESTINATION include/hello) 