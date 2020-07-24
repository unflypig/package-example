#cmake最低版本号
cmake_minimum_required(VERSION 2.6)
#工程名称
PROJECT(hello)
#源文件列表
SET(SOURCES main.c)
#依赖的库
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -luci")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -luci")
#生成可执行文件
ADD_EXECUTABLE(hello ${SOURCES})
#安装指令
INSTALL(TARGETS hello RUNTIME DESTINATION bin)
