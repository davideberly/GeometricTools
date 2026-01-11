# Define the project
project(gtmathematics
    VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR}
    DESCRIPTION "A header-only mathematics library"
    LANGUAGES CXX
)

# Minimum CMake version
cmake_minimum_required(VERSION 3.14)

include(GNUInstallDirs)

# Define the library as INTERFACE, which is typical for header-only libraries
add_library(gtmathematics INTERFACE)

# Set the include directories where the headers are located
# Assuming headers are in an "include" folder next to this CMakeLists.txt
target_include_directories(gtmathematics
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

# Installation support
include(CMakePackageConfigHelpers)

# Find all .h files in the same directory as this CMakeLists.txt
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# Install the header files
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/Mathematics)

# Install the target (export for downstream use)
install(
    TARGETS gtmathematics
    EXPORT gtmathematicsTargets
    INCLUDES DESTINATION include
)

# Generate and install the CMake package config file
install(EXPORT gtmathematicsTargets
    FILE gtmathematicsTargets.cmake
    NAMESPACE GTE::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gtmathematics
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/gtmathematicsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gtmathematicsConfig.cmake.in" # Template file
    "${CMAKE_CURRENT_BINARY_DIR}/gtmathematicsConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gtmathematics
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/gtmathematicsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/gtmathematicsConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gtmathematics
)

# Configure the export for a local build for find_package() to work
export(EXPORT gtmathematicsTargets
    FILE gtmathematicsTargets.cmake
    NAMESPACE GTE::
)