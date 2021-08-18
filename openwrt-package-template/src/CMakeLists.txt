#cmake最低版本号
cmake_minimum_required(VERSION 2.8.12)
#工程名称
SET(BIN_NAME openwrt-package-template)
PROJECT(${BIN_NAME})
#源文件列表
SET(SRC_LIST main.c
    log.c
    msgHandel.c
    common.c
    cJSON.c)
SET(SOURCES ${SRC_LIST})
#头文件路径
INCLUDE_DIRECTORIES(./inc
                    ./inc/mbedtls
                    )
#库文件路径
LINK_DIRECTORIES(./lib)

#生成可执行文件
ADD_EXECUTABLE(${BIN_NAME} ${SOURCES})
#依赖的库
TARGET_LINK_LIBRARIES(${BIN_NAME} mqttclient)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -luci")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -luci")
#安装指令
INSTALL(TARGETS ${BIN_NAME} RUNTIME DESTINATION bin)
