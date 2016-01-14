# cmake version
cmake_minimum_required(VERSION 2.8)
include($ENV{CMAKE_PATH}/func/func.cmake)

# set include and lib path
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/include $ENV{C_INCLUDE_PATH})
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib)

# set c compile parameters
ADD_DEFINITIONS(-Wall -O2 -Werror -g)

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
SET(POOL_SRC pool/pool.c)
SET(POOL_TEST_SRC pool/sample.c)

# lib generation
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/lib)
ADD_LIBRARY(thread SHARED ${LIB_SRC})
ADD_LIBRARY(mutex STATIC ${MUTEX_SRC})
ADD_LIBRARY(cond STATIC ${COND_SRC})
ADD_LIBRARY(bsem SHARED ${BSEM_SRC})
ADD_LIBRARY(thread_value SHARED ${VALUE_SRC})
ADD_LIBRARY(thread_rwlock SHARED ${RWLOCK_SRC})
ADD_LIBRARY(pool SHARED ${POOL_SRC})

# generat exec
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
ADD_EXECUTABLE(thread_bin ${CSRC})
ADD_EXECUTABLE(pool_test ${POOL_TEST_SRC})

# link library
SET_TARGET_PROPERTIES(thread PROPERTIES OUTPUT_NAME "thread")
TARGET_LINK_LIBRARIES(thread mutex pthread linked_list thread_value)
TARGET_LINK_LIBRARIES(bsem mutex cond)
TARGET_LINK_LIBRARIES(thread_rwlock mutex cond thread)
TARGET_LINK_LIBRARIES(thread_bin thread)
TARGET_LINK_LIBRARIES(pool thread linked_list bsem mutex)
TARGET_LINK_LIBRARIES(pool_test pool thread linked_list bsem mutex)

# -------------- INSTALL -------------- #
# install path setting
SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "$ENV{USR_TOOLS_PATH}" CACHE PATH "default install path" FORCE)
endif()

# install header files
install_hfiles(include include/thread)

# install library
install_lib(lib lib)

# install target
# INSTALL(TARGETS thread RUNTIME DESTINATION bin)
pre_install(pool)
