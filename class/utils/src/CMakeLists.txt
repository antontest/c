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
SET(CSRC           ${SRC_PATH})
SET(LIST_SRC       linked_list/linked_list.c)
SET(UTILS_SRC      utils/utils.c)
SET(ENUM_SRC       enum/enum.c)
SET(DEBUG_SRC      debug/debug.c)
SET(ARGS_SRC       get_args/get_args.c)
SET(LOG_SRC        log/log.c)
SET(QUEUE_SRC      queue/queue.c)
SET(QUEUE_TST      queue/sample.c)
SET(ELE_SRC        element/element.c)
SET(STACK_SRC      stack/stack.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(element     STATIC ${ELE_SRC})
ADD_LIBRARY(linked_list SHARED ${LIST_SRC})
ADD_LIBRARY(stack       STATIC ${STACK_SRC})
ADD_LIBRARY(queue       STATIC ${QUEUE_SRC})
ADD_LIBRARY(utils       SHARED ${UTILS_SRC})
ADD_LIBRARY(enum        STATIC ${ENUM_SRC})
ADD_LIBRARY(utils_debug STATIC ${DEBUG_SRC})
ADD_LIBRARY(get_args    STATIC ${ARGS_SRC})
ADD_LIBRARY(log         STATIC ${LOG_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(utils_bin ${CSRC})

# link library
#SET_TARGET_PROPERTIES(utils_share PROPERTIES OUTPUT_NAME "utils")
TARGET_LINK_LIBRARIES(utils_bin enum utils utils_debug log get_args)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{USR_TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install header files
install_hfiles(include include/utils)

# install library
install_lib(lib lib)

# install target
#INSTALL(TARGETS linked_list RUNTIME DESTINATION bin)
build_install(linked_list)
