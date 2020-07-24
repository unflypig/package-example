cmake_minimum_required(VERSION 2.6)
PROJECT(hello)
SET(SOURCES main.c)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -luci")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -luci")
#add_compile_options(-luci)
ADD_EXECUTABLE(hello ${SOURCES})
 
INSTALL(TARGETS hello RUNTIME DESTINATION bin)
