cmake_minimum_required(VERSION 2.8)
#set(CMAKE_INSTALL_PREFIX_INITALIZED_TO_DEFAULT 1)
if(CMAKE_INSTALL_PREFIX_INITALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX $TOOLS_PATH CACHE PATH "default install path" FORCE)
endif()

#定义工程明
PROJECT(TEST)

include($ENV{CMAKE_PATH}/func/func.cmake)
get_lib_by_path("src/lib/libdir.a" ccc)
message(STATUS "ccc: ${ccc}")
find_files(a src cc)
message(STATUS "cc: ${cc}")

#ADD_SUBDIRECTORY告诉cmake去子目录找CMakeLists.txt文件
#其中：src为子目录，bin是执行文件的生成目录
ADD_SUBDIRECTORY(src bin)
