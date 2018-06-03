cmake_minimum_required (VERSION 2.6)
project (CXCommunicationFramework)


# The version number.
set (CXCommunicationFramework_VERSION_MAJOR 1)
set (CXCommunicationFramework_VERSION_MINOR 0)

SET(CMAKE_BUILD_TYPE "Debug")


# Add the source in project root directory

#aux_source_directory(. DIRSRCS)

# Add header file include directories
# Add block directories
add_subdirectory(CXCommon)
add_subdirectory(CXEvent)
add_subdirectory(CXLock)
add_subdirectory(CXMemoryCache)
add_subdirectory(CXSocketWrapper)
add_subdirectory(CXThread)
add_subdirectory(CXLog)
add_subdirectory(CXFile)
add_subdirectory(CXContainer)
add_subdirectory(CXCommunicationClient)
add_subdirectory(CXCommunicationServer)
add_subdirectory(CXCommunicationServerTest)


# Target

add_executable(CXCommunicationFramework main.cpp)




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

