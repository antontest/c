# cmake version
cmake_minimum_required(VERSION 2.8)

# set include path
INCLUDE_DIRECTORIES($INCLUDE_PATH ${PROJECT_SOURCE_DIR}/src/include)

# set lib path
LINK_DIRECTORIES($LIB_PATH ${PROJECT_SOURCE_DIR}/src/lib)

# add source path
AUX_SOURCE_DIRECTORY(main SRC_PATH)

# set source files and lib source files
SET(CSRC ${SRC_PATH})
# SET(LIBSRC hello/hello.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
#ADD_LIBRARY(hello STATIC ${LIBSRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(main ${CSRC})

# link library
TARGET_LINK_LIBRARIES(main socket_app sctp)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX /home/anton/tools/)

# install lib
# INSTALL(FILES include/hello.h DESTINATION include)
# INSTALL(TARGETS hello ARCHIVE DESTINATION lib)

# install target
# INSTALL(TARGETS main RUNTIME DESTINATION exec)
