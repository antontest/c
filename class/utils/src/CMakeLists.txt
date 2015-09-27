# cmake version
cmake_minimum_required(VERSION 2.8)
include($ENV{CMAKE_PATH}/func/func.cmake)

# set include and lib path
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/include)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib)

# set c compile parameters
ADD_DEFINITIONS(-Wall -O2 -Werror)

# add source path
AUX_SOURCE_DIRECTORY(main SRC_PATH)

# set source files and lib source files
SET(CSRC ${SRC_PATH})
SET(LIST_SRC linked_list/linked_list.c)
SET(UTILS_SRC utils/utils.c)
SET(ENUM_SRC enum/enum.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(linked_list SHARED ${LIST_SRC})
ADD_LIBRARY(utils_share SHARED ${UTILS_SRC})
ADD_LIBRARY(enum SHARED ${ENUM_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(utils ${CSRC})

# link library
SET_TARGET_PROPERTIES(utils_share PROPERTIES OUTPUT_NAME "utils")
TARGET_LINK_LIBRARIES(utils linked_list enum utils_share)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install header files
INSTALL(FILES include/library.h DESTINATION include/utils)
INSTALL(FILES include/linked_list.h DESTINATION include/utils)
INSTALL(FILES include/utils.h DESTINATION include/utils)
INSTALL(FILES include/enum.h DESTINATION include/utils)

# install library
INSTALL(TARGETS linked_list LIBRARY DESTINATION lib)
INSTALL(TARGETS utils_share LIBRARY DESTINATION lib)
INSTALL(TARGETS enum LIBRARY DESTINATION lib)

# install target
#INSTALL(TARGETS linked_list RUNTIME DESTINATION exec)