cmake_minimum_required(VERSION 3.10)

# set the project name
project(pgi VERSION 1.0.1
            DESCRIPTION "Header only c++ library providing high level methods to interface C++ with postgres.")
add_compile_options(-std=c++17 -g -Wall -Wextra -pedantic)

add_library(pgi INTERFACE)
target_link_libraries(pgi INTERFACE -lpqxx -lpq -lyaml-cpp)

target_include_directories(pgi INTERFACE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include/pgi>)

# ========================================================== #
# Set helper variables for creating the version, config and target files.
include(CMakePackageConfigHelpers)
set(PGI_CMAKE_DIR "share/cmake/pgi" CACHE STRING
      "Installation directory for cmake files, relative to ${CMAKE_INSTALL_PREFIX}.")
set(VERSION_CONFIG "${PROJECT_BINARY_DIR}/pgi-config-version.cmake")
set(PROJECT_CONFIG "${PROJECT_BINARY_DIR}/pgi-config.cmake")
set(TARGETS_EXPORT_NAME pgi-targets)

# Generate the version, config and target files into the build directory.
write_basic_package_version_file(${VERSION_CONFIG}
    VERSION ${VERSION}
    COMPATIBILITY SameMajorVersion)
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/pgi-config.cmake.in
    ${PROJECT_CONFIG}
    INSTALL_DESTINATION ${PGI_CMAKE_DIR})
export(TARGETS pgi FILE ${PROJECT_BINARY_DIR}/${TARGETS_EXPORT_NAME}.cmake)

# Install version, config and target files.
install(FILES ${PROJECT_CONFIG} ${VERSION_CONFIG}
    DESTINATION ${PGI_CMAKE_DIR})
install(EXPORT ${TARGETS_EXPORT_NAME} DESTINATION ${PGI_CMAKE_DIR})

# Export the targets and install the header files.
install(TARGETS pgi EXPORT ${TARGETS_EXPORT_NAME} DESTINATION lib)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/include/pgi")

