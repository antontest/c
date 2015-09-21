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
SET(CSRC ${SRC_PATH})
SET(LIB_SRC queue/queue.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(queue_static STATIC ${LIB_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(queue ${CSRC})

# link library
SET_TARGET_PROPERTIES(queue_static PROPERTIES OUTPUT_NAME "queue")
TARGET_LINK_LIBRARIES(queue queue_static)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install header files
# INSTALL(FILES include/main.h DESTINATION include)
find_files(h include HLIST)
foreach(hfile ${HLIST})
    INSTALL(FILES ${hfile} DESTINATION include)
endforeach()

# install library
# INSTALL(TARGETS queue_static ARCHIVE DESTINATION lib)
find_files(aa lib LIBLIST)
if (LIBLIST)
    foreach(libfile ${LIBLIST})
        INSTALL(FILES ${libfile} DESTINATION lib)
    endforeach()
else()
    INSTALL(TARGETS queue_static ARCHIVE DESTINATION lib)
endif()

# install target
INSTALL(TARGETS queue RUNTIME DESTINATION exec)
