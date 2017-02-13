cmake_minimum_required(VERSION 3.6)
project(nlz)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_BUILD_TYPE distribution)
SET(CMAKE_CXX_FLAGS_DISTRIBUTION "-O3")
SET(CMAKE_C_FLAGS_DISTRIBUTION "-O3")
set(SOURCE_FILES main.cpp)
# set(BOOST_ROOT /usr/local/boost_1_63_0)
add_executable(nlz ${SOURCE_FILES})
# see https://cmake.org/cmake/help/latest/module/FindBoost.html
# find_package(Boost REQUIRED)

# include_directories(${Boost_INCLUDE_DIR})