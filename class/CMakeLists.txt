# version
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

#定义工程明
PROJECT(CLASS)
SET(PRE_INSTALL "1")

#ADD_SUBDIRECTORY告诉cmake去子目录找CMakeLists.txt文件
#其中：src为子目录，bin是执行文件的生成目录
ADD_SUBDIRECTORY(utils     utils_bin)
ADD_SUBDIRECTORY(cstring   cstring_bin)
ADD_SUBDIRECTORY(cint      cint_bin)
ADD_SUBDIRECTORY(unit_test unit_test_bin)
#ADD_SUBDIRECTORY(data    data_bin)
ADD_SUBDIRECTORY(proc      proc_bin)
ADD_SUBDIRECTORY(fileio    fileio_bin)
ADD_SUBDIRECTORY(dir       dir_bin)
ADD_SUBDIRECTORY(cgi       cgi_bin)
ADD_SUBDIRECTORY(thread    thread_bin)
ADD_SUBDIRECTORY(timer     timer_bin)
ADD_SUBDIRECTORY(tm        tm_bin)
ADD_SUBDIRECTORY(socket    socket_bin)
ADD_SUBDIRECTORY(sqlite    sqlite_bin)
