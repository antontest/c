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
SET(RWLOCK_SRC rwlock/rwlock.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(thread_share SHARED ${LIB_SRC})
ADD_LIBRARY(mutex STATIC ${MUTEX_SRC})
ADD_LIBRARY(cond STATIC ${COND_SRC})
ADD_LIBRARY(bsem SHARED ${BSEM_SRC})
ADD_LIBRARY(thread_value SHARED ${VALUE_SRC})
ADD_LIBRARY(thread_rwlock SHARED ${RWLOCK_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(thread ${CSRC})

# link library
SET_TARGET_PROPERTIES(thread_share PROPERTIES OUTPUT_NAME "thread")
TARGET_LINK_LIBRARIES(thread_share mutex pthread linked_list utils thread_value)
TARGET_LINK_LIBRARIES(bsem mutex cond utils)
TARGET_LINK_LIBRARIES(thread_rwlock mutex cond thread_share)
TARGET_LINK_LIBRARIES(thread thread_share)

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
# INSTALL(TARGETS thread_share ARCHIVE DESTINATION lib)
find_files(aa lib LIBLIST)
if (LIBLIST)
    foreach(libfile ${LIBLIST})
        INSTALL(FILES ${libfile} DESTINATION lib)
    endforeach()
endif()
INSTALL(TARGETS thread_share LIBRARY DESTINATION lib)
INSTALL(TARGETS thread_value LIBRARY DESTINATION lib)
INSTALL(TARGETS thread_rwlock LIBRARY DESTINATION lib)
INSTALL(TARGETS bsem LIBRARY DESTINATION lib)
INSTALL(TARGETS cond ARCHIVE DESTINATION lib)
INSTALL(TARGETS mutex ARCHIVE DESTINATION lib)

# install target
# INSTALL(TARGETS thread RUNTIME DESTINATION exec)