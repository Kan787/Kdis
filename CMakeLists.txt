#CMAKE最低版本要求
cmake_minimum_required(VERSION 3.0)
project(kdis)
# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#查找当前目录下的所有源文件并保存到DIR_SRCS变量里面
aux_source_directory(./stress_test DIR_SRCS)
#生成可执行文件
add_executable(stress_test ${DIR_SRCS})
TARGET_LINK_LIBRARIES(stress_test pthread) # 链接 <pthread.h>库，要在生成可执行文件的后面