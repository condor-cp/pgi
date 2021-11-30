cmake_minimum_required(VERSION 3.10)

# set the project name
project(pgi)
add_compile_options(-std=c++17 -g -Wall -Wextra -pedantic)

set(PGI_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config/")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config_dir.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/config_dir.hpp)

list(APPEND list_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/utl)

file(GLOB cpp_source_files
    "pgi.hpp"
)
add_library(pgi ${cpp_source_files})
set_target_properties(pgi PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(pgi PUBLIC ${list_include_dirs})
target_link_libraries(pgi PUBLIC -lpqxx -lpq -lyaml-cpp)

add_executable(main main.cpp)
target_link_libraries(main pgi)


