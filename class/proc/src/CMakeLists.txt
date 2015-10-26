# cmake version
cmake_minimum_required(VERSION 2.8)

# set include and lib path
INCLUDE_DIRECTORIES($ENV{INCLUDE_PATH} ${PROJECT_SOURCE_DIR}/src/include)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib)

# set c compile parameters
ADD_DEFINITIONS(-Wall -O2 -Werror -g -Wno-error=unused-result)

# add source path
AUX_SOURCE_DIRECTORY(main SRC_PATH)

# set source files and lib source files
SET(CSRC ${SRC_PATH})
SET(LIB_SRC proc/proc.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(proc_shared SHARED ${LIB_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(proc ${CSRC})

# link library
SET_TARGET_PROPERTIES(proc_shared PROPERTIES OUTPUT_NAME "proc")
TARGET_LINK_LIBRARIES(proc proc_shared)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{USR_TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install header files
INSTALL(FILES include/proc.h DESTINATION include)
INSTALL(FILES include/macro.h DESTINATION include)
    
# install library
INSTALL(TARGETS proc_shared LIBRARY DESTINATION lib)

# install target
INSTALL(TARGETS proc RUNTIME DESTINATION bin)
