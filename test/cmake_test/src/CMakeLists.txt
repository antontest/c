# cmake version
cmake_minimum_required(VERSION 3.1)

# set include path
INCLUDE_DIRECTORIES(${INCLUDE_PATH} include)

# set lib path
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O2 -Werror")

# add source path
AUX_SOURCE_DIRECTORY(main SRC_PATH)
AUX_SOURCE_DIRECTORY(t T_PATH)

# set source files and lib source files
SET(CSRC ${SRC_PATH})
SET(TSRC ${T_PATH})
SET(LIBSRC hello/hello.c)
FILE(GLOB_RECURSE LIBS ${PROJECT_SOURCE_DIR}/src/lib/*.a)
if(LIBS)
    message(STATUS "libs: ${LIBS}")
    STRING(REGEX REPLACE .*lib\(.*\).a$ \\1 liby ${LIBS})
endif()
if(liby)
    message(STATUS "liby: ${liby}")
endif()

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(hello STATIC ${LIBSRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(main ${CSRC})
ADD_EXECUTABLE(t ${TSRC})

# link library
ADD_DEPENDENCIES(main hello)
TARGET_LINK_LIBRARIES(main hello)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()
# SET(CMAKE_INSTALL_PREFIX "/home/anton/tools/")

# install lib
INSTALL(FILES include/hello.h DESTINATION include)
INSTALL(TARGETS hello ARCHIVE DESTINATION lib)

# install target
INSTALL(TARGETS main t RUNTIME DESTINATION exec)
