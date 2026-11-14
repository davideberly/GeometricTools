cmake_minimum_required(VERSION 3.30...4.4)

project(gtmathematicsgpu
	VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR}
	DESCRIPTION "GTE Mathematics library using the GPU"
	LANGUAGES CXX
)

set(gtmathematicsgpu_CPP_FILES
	GPUFluid2.cpp
	GPUFluid2AdjustVelocity.cpp
	GPUFluid2ComputeDivergence.cpp
	GPUFluid2EnforceStateBoundary.cpp
	GPUFluid2InitializeSource.cpp
	GPUFluid2InitializeState.cpp
	GPUFluid2SolvePoisson.cpp
	GPUFluid2UpdateState.cpp
	GPUFluid3.cpp
	GPUFluid3AdjustVelocity.cpp
	GPUFluid3ComputeDivergence.cpp
	GPUFluid3EnforceStateBoundary.cpp
	GPUFluid3InitializeSource.cpp
	GPUFluid3InitializeState.cpp
	GPUFluid3SolvePoisson.cpp
	GPUFluid3UpdateState.cpp
	GTMathematicsGPU.cpp
)

if(BUILD_SHARED_LIB)
	add_library(${PROJECT_NAME} SHARED ${gtmathematicsgpu_CPP_FILES})
	set_target_properties(${PROJECT_NAME}
		PROPERTIES
			SOVERSION ${PROJECT_VERSION_MAJOR}
			VERSION ${PROJECT_VERSION}
	)
else()
	add_library(${PROJECT_NAME} STATIC ${gtmathematicsgpu_CPP_FILES})
	set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
endif()

file(GLOB_RECURSE gtmathematicsgpu_HEADER_FILES
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)
target_sources(${PROJECT_NAME}
	INTERFACE
	FILE_SET fs_gtmathematicsgpu_headers
	TYPE
	HEADERS
	BASE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/..
	FILES
	${gtmathematicsgpu_HEADER_FILES}
)

add_library(GTE::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(${PROJECT_NAME} PUBLIC gte_usage_requirements)
target_link_libraries(${PROJECT_NAME} PRIVATE gte_warning_levels)

target_link_libraries(${PROJECT_NAME} PUBLIC GTE::gtmathematics GTE::gtgraphics)
