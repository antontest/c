# cmake version
cmake_minimum_required(VERSION 2.8)

# set include and lib path
INCLUDE_DIRECTORIES($INCLUDE_PATH ${PROJECT_SOURCE_DIR}/src/include)
LINK_DIRECTORIES($LIB_PATH ${PROJECT_SOURCE_DIR}/src/lib)

# set c compile parameters
ADD_DEFINITIONS(-Wall -O2 -Werror -g)

# set source files and lib source files
AUX_SOURCE_DIRECTORY(main SRC_PATH)
SET(CSRC ${SRC_PATH})
# SET(BASE_SRC socket_base/socket_base.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
# ADD_LIBRARY(socket_base STATIC ${BASE_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(cli ${CSRC})

# link library
# TARGET_LINK_LIBRARIES(socket_app socket_base socket_property pthread)
TARGET_LINK_LIBRARIES(cli socket_app)

# -------------- INSTALL -------------- #
# install path setting
# SET(CMAKE_INSTALL_PREFIX /home/anton/tools/)

# install header files
# INSTALL(FILES socket_base/socket_base.h DESTINATION include/socket)
    
# install library
# INSTALL(TARGETS socket_base ARCHIVE DESTINATION lib)

# install target
# INSTALL(TARGETS main RUNTIME DESTINATION exec)
