# version
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

#定义工程明
PROJECT(TT)

#ADD_SUBDIRECTORY告诉cmake去子目录找CMakeLists.txt文件
#其中：src为子目录，bin是执行文件的生成目录
ADD_SUBDIRECTORY(cmd_agent cmd_agent_bin)
ADD_SUBDIRECTORY(task_manager task_manager_bin)
ADD_SUBDIRECTORY(watchdog watchdog_bin)
