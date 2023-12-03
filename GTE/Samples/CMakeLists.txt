if(COMMAND cmake_policy)
    # Allow VERSION in the project() statement.
    cmake_policy(SET CMP0048 NEW)
endif()

set(GTE_VERSION_MAJOR 6)
set(GTE_VERSION_MINOR 8)

project(Samples VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR})

cmake_minimum_required(VERSION 3.8)
add_subdirectory(Distance)
add_subdirectory(Geometrics)
add_subdirectory(Graphics)
add_subdirectory(Imagics)
add_subdirectory(Intersection)
add_subdirectory(Mathematics)
add_subdirectory(Physics)
add_subdirectory(SceneGraphs)
