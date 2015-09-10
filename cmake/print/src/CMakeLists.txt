# cmake version
cmake_minimum_required(VERSION 3.1)
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
SET(LIB_SRC print/print.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(print_static STATIC ${LIB_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(print ${CSRC})

# link library
SET_TARGET_PROPERTIES(print_static PROPERTIES OUTPUT_NAME "print")
TARGET_LINK_LIBRARIES(print print_static)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install header files
find_files(h include HLIST)
message(STATUS "${HLIST}")
foreach(hfile ${HLIST})
    INSTALL(FILES ${hfile} DESTINATION include)
endforeach()

# install library
INSTALL(TARGETS print_static ARCHIVE DESTINATION lib)

# install target
# INSTALL(TARGETS print RUNTIME DESTINATION exec)
