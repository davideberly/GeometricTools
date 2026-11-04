# Minimum CMake version
cmake_minimum_required(VERSION 3.30...4.4)

# Define the project
project(gtmathematics
	VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR}
	DESCRIPTION "A header-only mathematics library"
	LANGUAGES CXX
)

include(GNUInstallDirs)

# Define the library as INTERFACE, which is typical for header-only libraries
add_library(gtmathematics INTERFACE)
add_library(GTE::gtmathematics ALIAS gtmathematics)

# Installation support
include(CMakePackageConfigHelpers)

# Find all .h files in the same directory as this CMakeLists.txt
file(GLOB gtmathematics_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
target_sources(gtmathematics
	INTERFACE
	FILE_SET fs_gtmathematics_headers
	TYPE
	HEADERS
	BASE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/..
	FILES
	${gtmathematics_HEADER_FILES}
)

target_link_libraries(gtmathematics INTERFACE gte_usage_requirements)

# Install the target (export for downstream use)
install(
	TARGETS
		gtmathematics
		gte_usage_requirements
	EXPORT gtmathematicsTargets
	FILE_SET fs_gtmathematics_headers
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
)

# Generate and install the CMake package config file
install(EXPORT gtmathematicsTargets
	FILE gtmathematicsTargets.cmake
	NAMESPACE GTE::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gtmathematics
)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/gtmathematicsConfigVersion.cmake"
	VERSION
	${PROJECT_VERSION}
	COMPATIBILITY
	SameMajorVersion
)

configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/gtmathematicsConfig.cmake.in"
	# Template file
	"${CMAKE_CURRENT_BINARY_DIR}/gtmathematicsConfig.cmake"
	INSTALL_DESTINATION
	${CMAKE_INSTALL_LIBDIR}/cmake/gtmathematics
)

install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/gtmathematicsConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/gtmathematicsConfigVersion.cmake"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gtmathematics
)

# Configure the export for a local build for find_package() to work
export(
	EXPORT
	gtmathematicsTargets
	FILE gtmathematicsTargets.cmake
	NAMESPACE GTE::
)