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
SET(LIB_SRC socket/socket.c)
SET(HOST_SRC host/host.c)
SET(MANAGER_SRC socket_manager/socket_manager.c)
SET(CHUNK_SRC chunk/chunk.c)
SET(PACKET_SRC packet/packet.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(chunk SHARED ${CHUNK_SRC})
ADD_LIBRARY(packet SHARED ${PACKET_SRC})
ADD_LIBRARY(socket_shared SHARED ${LIB_SRC})
ADD_LIBRARY(host SHARED ${HOST_SRC})
ADD_LIBRARY(socket_manager SHARED ${MANAGER_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(socket ${CSRC})

# link library
SET_TARGET_PROPERTIES(socket_shared PROPERTIES OUTPUT_NAME "socket")
TARGET_LINK_LIBRARIES(host utils)
TARGET_LINK_LIBRARIES(chunk utils)
TARGET_LINK_LIBRARIES(packet utils chunk)
TARGET_LINK_LIBRARIES(socket_shared utils packet chunk host thread enum mutex)
TARGET_LINK_LIBRARIES(socket_manager utils)
TARGET_LINK_LIBRARIES(socket socket_shared)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{USR_TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install header and library files
install_hfiles(include include/socket)
install_lib(lib lib)

# install target
# INSTALL(TARGETS socket RUNTIME DESTINATION bin)
