############################
### PROJECT SETUP
############################
cmake_minimum_required(VERSION 3.14...4.4)

if(COMMAND cmake_policy)
	# Allow VERSION in the project() statement.
	cmake_policy(SET CMP0048 NEW)
endif()

set(GTE_VERSION_MAJOR 8)
set(GTE_VERSION_MINOR 2)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(GTE
	HOMEPAGE_URL "https://www.geometrictools.com/"
	VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR}
	LANGUAGES CXX
)

############################
### PROJECT OPTIONS
############################
option(GTE_USE_DIRECTX "Use DirectX backend (only on Windows)" OFF)
if(GTE_USE_DIRECTX)
	set(GTE_USE_OPENGL OFF)
else()
	set(GTE_USE_OPENGL ON)
endif()

option(GTE_BUILD_SHARED_LIBS "Build shared libraries" ${BUILD_SHARED_LIBS})

option(GTE_BUILD_GRAPHICS "Build Graphics/ subdirectory" ON)
option(GTE_BUILD_MATHEMATICS_GPU "Build MathematicsGPU/ subdirectory" ON)
option(GTE_BUILD_APPLICATIONS "Build Applications/ subdirectory" ON)
option(GTE_BUILD_SAMPLES "Build Samples/ subdirectory" OFF)

option(GTE_STAGE_OUTPUTS
	"Stage build output artifacts into proper directory structure"
	ON
)
if(GTE_STAGE_OUTPUTS)
	include(cmake/incom/StageOutputs.cmake)
	if(GTE_BUILD_SHARED_LIBS)
		stageOutputDirsFor(GTE ${CMAKE_BUILD_TYPE}Shared)
	else()
		stageOutputDirsFor(GTE ${CMAKE_BUILD_TYPE}Static)
	endif()
endif()

if(NOT GTE_BUILD_GRAPHICS)
	set(GTE_BUILD_MATHEMATICS_GPU OFF)
endif()
if(NOT GTE_BUILD_MATHEMATICS_GPU)
	set(GTE_BUILD_APPLICATIONS OFF)
endif()
if(NOT GTE_BUILD_APPLICATIONS)
	set(GTE_BUILD_SAMPLES OFF)
endif()

###########################################
### DEALING WITH PLATFORM DIFFERENCES
###########################################
if(GTE_USE_DIRECTX AND NOT WIN32)
	message(FATAL_ERROR
		"Attempting to use DirectX on non-Windows platform. That is impossible."
	)
endif()

# Usage requirements of the libraries (mostly defines)
add_library(gte_usage_requirements INTERFACE)
target_compile_definitions(gte_usage_requirements
	INTERFACE
		GTE_USE_ROW_MAJOR
		GTE_USE_MAT_VEC
		GTE_DISABLE_PCH
)
if(WIN32)
	target_compile_definitions(gte_usage_requirements
		INTERFACE
			GTE_USE_MSWINDOWS
			CONSOLE
			_CONSOLE
			UNICODE
			_UNICODE
			NOMINMAX
	)
else()
	target_compile_definitions(gte_usage_requirements INTERFACE GTE_USE_LINUX)
endif()

if(GTE_USE_DIRECTX)
	target_compile_definitions(gte_usage_requirements INTERFACE GTE_USE_DIRECTX)
else()
	target_compile_definitions(gte_usage_requirements INTERFACE GTE_USE_OPENGL)
endif()

# Warning levels for the libraries
add_library(gte_warning_levels INTERFACE)
set_target_properties(gte_warning_levels PROPERTIES COMPILE_WARNING_AS_ERROR ON)
if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC")
	target_compile_options(gte_warning_levels INTERFACE /W4)
elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "GNU")
	target_compile_options(gte_warning_levels INTERFACE -Wall -Wextra)
else()
	message(WARNING
		"Using compiler with a frontend that is neither MSVC-like nor GNU-like."
	)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

set(GTE_GRAPHICS_BACKEND_LIBRARIES "")
set(GTE_APPLICATIONS_BACKEND_LIBRARIES "")
if(WIN32)
	list(APPEND GTE_APPLICATIONS_BACKEND_LIBRARIES windowscodecs)
endif()
if(GTE_USE_DIRECTX)
	find_package(DirectX11 REQUIRED)
	list(APPEND GTE_GRAPHICS_BACKEND_LIBRARIES ${DirectX11_LIBRARIES})
elseif(GTE_USE_OPENGL)
	if(LINUX)
		find_package(OpenGL REQUIRED COMPONENTS EGL)
		find_package(PNG REQUIRED)
		find_package(X11 REQUIRED)
		find_package(Threads REQUIRED)
		list(APPEND GTE_GRAPHICS_BACKEND_LIBRARIES
			OpenGL::GL
			OpenGL::EGL
			X11::X11
			Threads::Threads
		)
		list(APPEND GTE_APPLICATIONS_BACKEND_LIBRARIES PNG::PNG X11::X11)
	elseif(WIN32)
		find_package(OpenGL REQUIRED)
		list(APPEND GTE_GRAPHICS_BACKEND_LIBRARIES OpenGL::GL)
	elseif(APPLE)
		message(FATAL_ERROR "Apple platform are not known to be supported.")
	else()
		message(FATAL_ERROR
			"Exotic platforms (non-Linux, non-Windows, non-Apple) are not known to be supported."
		)
	endif()
endif()

###########################################
### ADDING LIBRARIES INTO THE BUILD
###########################################
add_subdirectory(Mathematics)

if(GTE_BUILD_GRAPHICS)
	add_subdirectory(Graphics)
endif()

if(GTE_BUILD_APPLICATIONS)
	add_subdirectory(Applications)
endif()

if(GTE_BUILD_MATHEMATICS_GPU)
	add_subdirectory(MathematicsGPU)
endif()

if(GTE_BUILD_SAMPLES)
	add_subdirectory(Samples)
endif()