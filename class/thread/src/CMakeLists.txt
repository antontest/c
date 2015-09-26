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
SET(LIB_SRC thread/thread.c)
SET(MUTEX_SRC mutex/mutex.c)
SET(COND_SRC cond/cond.c)
SET(BSEM_SRC bsem/bsem.c)
SET(VALUE_SRC thread_value/thread_value.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(thread_static SHARED ${LIB_SRC})
ADD_LIBRARY(mutex STATIC ${MUTEX_SRC})
ADD_LIBRARY(cond STATIC ${COND_SRC})
ADD_LIBRARY(bsem SHARED ${BSEM_SRC})
ADD_LIBRARY(thread_value STATIC ${VALUE_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(thread ${CSRC})

# link library
SET_TARGET_PROPERTIES(thread_static PROPERTIES OUTPUT_NAME "thread")
TARGET_LINK_LIBRARIES(thread_static mutex pthread thread_value linked_list)
TARGET_LINK_LIBRARIES(bsem mutex cond)
TARGET_LINK_LIBRARIES(thread thread_static)

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
    INSTALL(FILES ${hfile} DESTINATION include/thread)
endforeach()

# install library
# INSTALL(TARGETS thread_static ARCHIVE DESTINATION lib)
find_files(aa lib LIBLIST)
if (LIBLIST)
    foreach(libfile ${LIBLIST})
        INSTALL(FILES ${libfile} DESTINATION lib)
    endforeach()
endif()
INSTALL(TARGETS thread_static LIBRARY DESTINATION lib)
INSTALL(TARGETS bsem LIBRARY DESTINATION lib)

# install target
# INSTALL(TARGETS thread RUNTIME DESTINATION exec)
