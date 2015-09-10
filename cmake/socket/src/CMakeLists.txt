# cmake version
cmake_minimum_required(VERSION 2.8)

# set include and lib path
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/include)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib)

# set c compile parameters
ADD_DEFINITIONS(-Wall -O2 -Werror -g)

# add source path
AUX_SOURCE_DIRECTORY(main SRC_PATH)

# set source files and lib source files
SET(CSRC ${SRC_PATH})
SET(BASE_SRC socket_base/socket_base.c)
SET(PROPERTY_SRC socket_property/socket_property.c)
SET(APP_SRC socket_app/socket_app.c socket_app/socket_event.c)
SET(MSG_SRC socket_msg/socket_msg.c)
SET(ARP_SRC socket_arp/socket_arp.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(socket_base STATIC ${BASE_SRC})
ADD_LIBRARY(socket_property STATIC ${PROPERTY_SRC})
ADD_LIBRARY(socket_app SHARED ${APP_SRC})
ADD_LIBRARY(socket_msg STATIC ${MSG_SRC})
ADD_LIBRARY(socket_arp SHARED ${ARP_SRC})
# ADD_LIBRARY(socket_app_static STATIC ${APP_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(socket ${CSRC})

# link library
ADD_DEPENDENCIES(socket_arp timer pthread pcap socket_proerty)
TARGET_LINK_LIBRARIES(socket_arp pthread pcap timer socket_property)
ADD_DEPENDENCIES(socket_app socket_base socket_property pthread pcap timer)
TARGET_LINK_LIBRARIES(socket_app socket_base socket_property pthread pcap timer)
# SET_TARGET_PROPERTIES(socket_app_static PROPERTIES OUTPUT_NAME "socket_app")
ADD_DEPENDENCIES(socket socket_app socket_arp)
TARGET_LINK_LIBRARIES(socket socket_app socket_arp)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install header files
INSTALL(FILES include/socket_base.h DESTINATION include/socket)
INSTALL(FILES include/socket_property.h DESTINATION include/socket)
INSTALL(FILES include/socket_app.h DESTINATION include/socket)
INSTALL(FILES include/socket_event.h DESTINATION include/socket)
INSTALL(FILES include/socket_header.h DESTINATION include/socket)
INSTALL(FILES include/socket_arp.h DESTINATION include/socket)
#INSTALL(FILES socket_msg/socket_msg.h DESTINATION include/socket)
    
# install library
INSTALL(TARGETS socket_base ARCHIVE DESTINATION lib)
INSTALL(TARGETS socket_property ARCHIVE DESTINATION lib)
INSTALL(TARGETS socket_app LIBRARY DESTINATION lib)
INSTALL(TARGETS socket_arp LIBRARY DESTINATION lib)
# INSTALL(TARGETS socket_msg ARCHIVE DESTINATION lib)
# INSTALL(TARGETS socket_app_static ARCHIVE DESTINATION lib)

# install target
INSTALL(TARGETS socket RUNTIME DESTINATION exec)
