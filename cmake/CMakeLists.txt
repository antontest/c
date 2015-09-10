# version
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

#定义工程明
PROJECT(CMAKE_LIB)

#ADD_SUBDIRECTORY告诉cmake去子目录找CMakeLists.txt文件
#其中：src为子目录，bin是执行文件的生成目录
ADD_SUBDIRECTORY(socket socket_bin)
ADD_SUBDIRECTORY(dir dir_bin)
ADD_SUBDIRECTORY(cfg cfg_bin)
ADD_SUBDIRECTORY(proc proc_bin)
ADD_SUBDIRECTORY(thread thread_bin)
ADD_SUBDIRECTORY(timer timer_bin)
ADD_SUBDIRECTORY(print print_bin)
