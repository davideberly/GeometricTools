if(COMMAND cmake_policy)
    # Allow VERSION in the project() statement.
    cmake_policy(SET CMP0048 NEW)
endif()

cmake_minimum_required(VERSION 3.8)

set(GTE_VERSION_MAJOR 6)
set(GTE_VERSION_MINOR 8)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

project(GTE VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR})

add_subdirectory(Applications)
add_subdirectory(Graphics)
add_subdirectory(Mathematics)
add_subdirectory(MathematicsGPU)

option(BUILD_SAMPLES "Build Samples" OFF)
if(BUILD_SAMPLES)
  add_subdirectory(Samples)
endif()