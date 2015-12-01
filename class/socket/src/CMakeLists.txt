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
SET(CSRC         ${SRC_PATH})
SET(LIB_SRC      socket/socket.c)
SET(HOST_SRC     host/host.c)
SET(MANAGER_SRC  manager/manager.c)
SET(CHUNK_SRC    chunk/chunk.c)
SET(PACKET_SRC   packet/packet.c)
SET(PROPERTY_SRC property/property.c)
SET(FTP_SRC      ftp/ftp.c)
SET(FTP_MAIN_SRC ftp/ftp-main.c)
SET(BASE_SRC     base/base.c)
SET(EVENT_SRC    event/event.c)
SET(EVENT_TEST   event/sample.c)
SET(PTEST_SRC    property/sample.c)
SET(CAST_SRC     cast/cast.c)
SET(CAST_TEST    cast/sample.c)
SET(LOCAL_SOCKET local/local.c)
SET(LOCAL_TEST   local/sample.c)
SET(MSG_SRC      message/message.c)
SET(MSG_TEST     message/sample.c)
SET(MSG_SEND     message/msg_send.c)
SET(LISTEN_SRC   listener/listener.c)
SET(LISTEN_TEST  listener/sample.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(chunk         SHARED ${CHUNK_SRC})
ADD_LIBRARY(packet        SHARED ${PACKET_SRC})
ADD_LIBRARY(socket_shared SHARED ${LIB_SRC})
ADD_LIBRARY(host          STATIC ${HOST_SRC})
ADD_LIBRARY(manager       SHARED ${MANAGER_SRC})
ADD_LIBRARY(property      STATIC ${PROPERTY_SRC})
ADD_LIBRARY(ftp           SHARED ${FTP_SRC})
ADD_LIBRARY(socket_base   SHARED ${BASE_SRC})
ADD_LIBRARY(event         SHARED ${EVENT_SRC})
ADD_LIBRARY(cast          SHARED ${CAST_SRC})
ADD_LIBRARY(local_socket  SHARED ${LOCAL_SOCKET})
ADD_LIBRARY(message       SHARED ${MSG_SRC})
ADD_LIBRARY(listener      SHARED ${LISTEN_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(socket ${CSRC})
ADD_EXECUTABLE(ftp_client ${FTP_MAIN_SRC})
ADD_EXECUTABLE(event_test ${EVENT_TEST})
ADD_EXECUTABLE(property_test ${PTEST_SRC})
ADD_EXECUTABLE(cast_test ${CAST_TEST})
ADD_EXECUTABLE(local_test ${LOCAL_TEST})
ADD_EXECUTABLE(message_test ${MSG_TEST})
ADD_EXECUTABLE(msg_send ${MSG_SEND})
ADD_EXECUTABLE(listener_test ${LISTEN_TEST})

# link library
SET_TARGET_PROPERTIES(socket_shared PROPERTIES OUTPUT_NAME "socket")
# TARGET_LINK_LIBRARIES(host          utils)
TARGET_LINK_LIBRARIES(chunk         utils)
TARGET_LINK_LIBRARIES(packet        utils chunk)
TARGET_LINK_LIBRARIES(socket_shared utils packet chunk host thread enum mutex socket_base)
TARGET_LINK_LIBRARIES(socket_base   host)
TARGET_LINK_LIBRARIES(manager       utils)
TARGET_LINK_LIBRARIES(ftp           fileio socket_shared)
TARGET_LINK_LIBRARIES(socket        socket_shared get_args property ftp socket_base)
TARGET_LINK_LIBRARIES(ftp_client    socket_shared get_args ftp)
TARGET_LINK_LIBRARIES(event         thread linked_list)
TARGET_LINK_LIBRARIES(event_test    socket_base event linked_list)
TARGET_LINK_LIBRARIES(property_test property)
TARGET_LINK_LIBRARIES(cast          socket_base)
TARGET_LINK_LIBRARIES(cast_test     socket_base cast utils property)
TARGET_LINK_LIBRARIES(local_test    local_socket)
TARGET_LINK_LIBRARIES(message       local_socket proc mutex bsem thread)
TARGET_LINK_LIBRARIES(message_test  local_socket proc mutex bsem thread message event)
TARGET_LINK_LIBRARIES(msg_send      local_socket proc mutex bsem thread message event)
TARGET_LINK_LIBRARIES(listener      socket_base event pthread chunk packet)
TARGET_LINK_LIBRARIES(listener_test socket_base event pthread listener)

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
INSTALL(TARGETS ftp_client RUNTIME DESTINATION bin)
