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
SET(LIB_SRC data/data.c)
SET(STACK_SRC stack/stack.c)
SET(LIST_SRC  linked_list/linked_list.c)
SET(QUEUE_SRC queue/queue.c)
SET(QUEUE_TST queue/sample.c)
SET(ELE_SRC   element/element.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(data_static STATIC ${LIB_SRC})
ADD_LIBRARY(stack       STATIC ${STACK_SRC})
ADD_LIBRARY(linked_list STATIC ${LIST_SRC})
ADD_LIBRARY(queue       STATIC ${QUEUE_SRC})
ADD_LIBRARY(element     STATIC ${ELE_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(data ${CSRC})
ADD_EXECUTABLE(queue_test ${QUEUE_TST})

# link library
SET_TARGET_PROPERTIES(data_static PROPERTIES OUTPUT_NAME "data")
TARGET_LINK_LIBRARIES(data data_static stack linked_list)
TARGET_LINK_LIBRARIES(queue_test queue)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{USR_TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install header and library files
install_hfiles(include include/data)
install_lib(lib lib)

# install target
# INSTALL(TARGETS data RUNTIME DESTINATION bin)
