# cmake version
cmake_minimum_required(VERSION 2.8)
include($ENV{CMAKE_PATH}/func/func.cmake)

# set include and lib path
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/include ../../utils/src/include/)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib)

# set c compile parameters
ADD_DEFINITIONS(-Wall -O2 -Werror)

# add source path
AUX_SOURCE_DIRECTORY(main SRC_PATH)

# set source files and lib source files
SET(CSRC ${SRC_PATH})
SET(LIB_SRC sqlite/sqlite.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(sqlite STATIC ${LIB_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(sqlite_bin ${CSRC})

# link library
SET_TARGET_PROPERTIES(sqlite_bin PROPERTIES OUTPUT_NAME "sqlite")
TARGET_LINK_LIBRARIES(sqlite sqlite3)
TARGET_LINK_LIBRARIES(sqlite_bin sqlite)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{USR_TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install header and library files
install_hfiles(include include)
install_lib(lib lib)

# install target
#INSTALL(TARGETS sqlite_bin RUNTIME DESTINATION bin)
