cmake_minimum_required(VERSION 3.16)
project(DBoW2)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall  -O3 -march=native ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall  -O3 -march=native")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

find_package(OpenCV 4 REQUIRED)


file(GLOB_RECURSE dbow2_files CONFIGURE_DEPENDS *.h *.cpp *.hpp)
add_library(DBoW2 SHARED ${dbow2_files})
target_include_directories(DBoW2 PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(DBoW2 ${OpenCV_LIBS})