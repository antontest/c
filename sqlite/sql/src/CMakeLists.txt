# cmake version
cmake_minimum_required(VERSION 2.8)

# set include and lib path
INCLUDE_DIRECTORIES($ENV{INCLUDE_PATH} ${PROJECT_SOURCE_DIR}/src/include)
LINK_DIRECTORIES($ENV{LIB_PATH} ${PROJECT_SOURCE_DIR}/src/lib)

# set c compile parameters
ADD_DEFINITIONS(-Wall -O2 -Werror -g)

# set source files and lib source files
AUX_SOURCE_DIRECTORY(main SRC_PATH)
AUX_SOURCE_DIRECTORY(lib LIB_PATH)
SET(CSRC ${SRC_PATH})
SET(LIBS ${LIB_PATH})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(sql ${CSRC})

# link library
# TARGET_LINK_LIBRARIES(sql sql_static)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install target
INSTALL(TARGETS sql RUNTIME DESTINATION exec)
