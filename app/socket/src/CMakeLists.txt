# cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
include($ENV{CMAKE_PATH}/func/func.cmake)

# set include and lib path
INCLUDE_DIRECTORIES($ENV{INCLUDE_PATH} ${PROJECT_SOURCE_DIR}/src/include)
LINK_DIRECTORIES($ENV{LIB_PATH} ${PROJECT_SOURCE_DIR}/src/lib)

# set c compile parameters
ADD_DEFINITIONS(-Wall -O2 -Werror -g)

# set source files 
AUX_SOURCE_DIRECTORY(main SRC_PATH)
SET(CSRC ${SRC_PATH})

# set lib source files
SET(EXTERNAL_LIBS socket host property get_args arp)
FILE(GLOB_RECURSE LIBS ${PROJECT_SOURCE_DIR}/src/extra_lib/*.a)
IF(LIBS)
    STRING(REGEX REPLACE .*lib\(.*\).a$ \\1 LIBS ${LIBS})
ENDIF()

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(socket_bin ${CSRC})
SET_TARGET_PROPERTIES(socket_bin PROPERTIES OUTPUT_NAME "socket")

# link library
IF(LIBS)
    TARGET_LINK_LIBRARIES(socket_bin ${LIBS})
ENDIF()
IF(EXTERNAL_LIBS)
    TARGET_LINK_LIBRARIES(socket_bin ${EXTERNAL_LIBS})
ENDIF()

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{USR_TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install target
INSTALL(TARGETS socket_bin RUNTIME DESTINATION bin)
