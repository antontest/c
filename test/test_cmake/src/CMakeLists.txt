#最低版本要求
cmake_minimum_required(VERSION 2.8)

#添加头文件路径
#其中：inlcude为头文件路径
INCLUDE_DIRECTORIES(include)

#指定库的连接路径
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib/)

#添加源文件路径
#注意不同同时添加多个源文件路径，只能如下一个路径一个路径的添加
AUX_SOURCE_DIRECTORY(main SRC_PATH)
#AUX_SOURCE_DIRECTORY(hello SRC_PATH)

#设置路径环境变量
SET(SRC ${SRC_PATH})

#添加文件执行
ADD_EXECUTABLE(main ${SRC})

#链接库文件
#TARGET_LINK_LIBRARIES(main myTCP)
