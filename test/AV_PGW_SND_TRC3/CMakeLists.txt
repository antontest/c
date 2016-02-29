# version
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

#定义工程明
PROJECT(AV_PGW_SND_TRC3)

# 指定编译器
if (DEFINED CC)
    if (${CC} STREQUAL "clang")
        SET(COMPILER_PATH "/home/anton/etc/clang")
        SET(CMAKE_C_COMPILER "${COMPILER_PATH}/bin/clang")
        SET(CMAKE_CXX_COMPILER "${COMPILER_PATH}/bin/clang++")
        SET(CMAKE_AR "${COMPILER_PATH}/bin/llvm-ar")
        SET(CMAKE_LINKER "${COMPILER_PATH}/bin/llvm-link")
        SET(CMAKE_NM "${COMPILER_PATH}/bin/llvm-nm")
        SET(CMAKE_OBJDUMP "${COMPILER_PATH}/bin/llvm-objdump")
        SET(CMAKE_RANLIB "${COMPILER_PATH}/bin/llvm-ranlib")
    endif(${CC} STREQUAL "clang")
endif(DEFINED CC)

#ADD_SUBDIRECTORY告诉cmake去子目录找CMakeLists.txt文件
#其中：src为子目录，etc是执行文件的生成目录
ADD_SUBDIRECTORY(src src_bin)
