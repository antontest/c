# version
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

#定义工程明
PROJECT(PROC)

# 指定编译器
SET(CMAKE_C_COMPILER "/home/anton/etc/clang/bin/clang")
SET(CMAKE_CXX_COMPILER "/home/anton/etc/clang/bin/clang++")
SET(CMAKE_AR "/home/anton/etc/clang/bin/llvm-ar")
SET(CMAKE_LINKER "/home/anton/etc/clang/bin/llvm-link")
SET(CMAKE_NM "/home/anton/etc/clang/bin/llvm-nm")
SET(CMAKE_OBJDUMP "/home/anton/etc/clang/bin/llvm-objdump")
SET(CMAKE_RANLIB "/home/anton/etc/clang/bin/llvm-ranlib")

#ADD_SUBDIRECTORY告诉cmake去子目录找CMakeLists.txt文件
#其中：src为子目录，etc是执行文件的生成目录
ADD_SUBDIRECTORY(src src_bin)
