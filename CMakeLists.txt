# version
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

#定义工程明
PROJECT(ALL)
SET(BUILD_INSTALL "1")

#ADD_SUBDIRECTORY告诉cmake去子目录找CMakeLists.txt文件
#其中：src为子目录，bin是执行文件的生成目录
ADD_SUBDIRECTORY(rc    rc_bin)
ADD_SUBDIRECTORY(cgi   cgi_bin)
ADD_SUBDIRECTORY(app   app_bin)
ADD_SUBDIRECTORY(class class_bin)
