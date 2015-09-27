# cmake version
cmake_minimum_required(VERSION 2.8)
include($ENV{CMAKE_PATH}/func/func.cmake)

# set include and lib path
INCLUDE_DIRECTORIES($ENV{INCLUDE_PATH} ${PROJECT_SOURCE_DIR}/src/include)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib)

# set c compile parameters
ADD_DEFINITIONS(-Wall -O2 -Werror)

# add source path
AUX_SOURCE_DIRECTORY(main SRC_PATH)

# set source files and lib source files
SET(CSRC ${SRC_PATH})
SET(LIB_SRC timer/timer.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(timer_share SHARED ${LIB_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(timer ${CSRC})

# link library
SET_TARGET_PROPERTIES(timer_share PROPERTIES OUTPUT_NAME "timer")
ADD_DEPENDENCIES(timer_share pthread mutex utils)
TARGET_LINK_LIBRARIES(timer_share pthread mutex utils)
TARGET_LINK_LIBRARIES(timer timer_share pthread)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install header files
# INSTALL(FILES include/main.h DESTINATION include)
find_files(h include HLIST)
foreach(hfile ${HLIST})
    INSTALL(FILES ${hfile} DESTINATION include)
endforeach()

# install library
INSTALL(TARGETS timer_share LIBRARY DESTINATION lib)

# install target
# INSTALL(TARGETS timer RUNTIME DESTINATION exec)
