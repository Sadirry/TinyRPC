cmake_minimum_required(VERSION 3.0)
project(Muduo)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")

aux_source_directory(./src SRC_LIST)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(Muduo SHARED ${SRC_LIST})