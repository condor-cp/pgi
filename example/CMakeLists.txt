cmake_minimum_required(VERSION 3.10)

set(CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config/")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config_dir.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/config_dir.hpp)

find_package(pgi CONFIG REQUIRED)

# set the project name
project(pgi_test)
add_compile_options(-std=c++17 -g -Wall -Wextra -pedantic) 

add_executable(main main.cpp)
target_link_libraries(main pgi)
