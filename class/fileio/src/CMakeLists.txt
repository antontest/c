# cmake version
cmake_minimum_required(VERSION 2.8)
include($ENV{CMAKE_PATH}/func/func.cmake)

# set include and lib path
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/include $ENV{C_INCLUDE_PATH} ../../utils/src/include/)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib)

# set c compile parameters
ADD_DEFINITIONS(-Wall -O2 -Werror)

# add source path
AUX_SOURCE_DIRECTORY(main SRC_PATH)

# set source files and lib source files
SET(CSRC ${SRC_PATH})
SET(LIB_SRC fileio/fileio.c dir/dir.c)
SET(INI_SRC fileio/ini.c)
SET(CFG_SRC fileio/cfg.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(fileio STATIC ${LIB_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(fileio_bin ${CSRC})
ADD_EXECUTABLE(ini        ${INI_SRC})
ADD_EXECUTABLE(cfg        ${CFG_SRC})

# link library
SET_TARGET_PROPERTIES(fileio_bin PROPERTIES OUTPUT_NAME "fileio")
TARGET_LINK_LIBRARIES(fileio_bin fileio)
TARGET_LINK_LIBRARIES(ini        fileio get_args)
TARGET_LINK_LIBRARIES(cfg        fileio get_args)

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
# INSTALL(TARGETS fileio RUNTIME DESTINATION bin)
build_install(fileio)
