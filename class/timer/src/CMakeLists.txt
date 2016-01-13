# cmake version
cmake_minimum_required(VERSION 2.8)
include($ENV{CMAKE_PATH}/func/func.cmake)

# set include and lib path
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/include ../../utils/src/include/ ../../thread/src/include/)
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
ADD_LIBRARY(timer SHARED ${LIB_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(timer_bin ${CSRC})

# link library
SET_TARGET_PROPERTIES(timer_bin PROPERTIES OUTPUT_NAME "timer")
TARGET_LINK_LIBRARIES(timer pthread mutex enum)
TARGET_LINK_LIBRARIES(timer_bin timer)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{USR_TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install header files
install_hfiles(include include)

# install library
install_lib(lib lib)

# install target
# INSTALL(TARGETS timer RUNTIME DESTINATION bin)
