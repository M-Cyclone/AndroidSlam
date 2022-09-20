cmake_minimum_required(VERSION 3.16)
project(g2o)

# default built type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -W")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -march=native")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")


file(GLOB_RECURSE g2o_files CONFIGURE_DEPENDS *.h *.cpp *.hpp)
add_library(g2o SHARED ${g2o_files})
target_include_directories(g2o
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/g2o/core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/g2o/types
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/g2o/stuff
    PUBLIC ${external_dir}/eigen3
)