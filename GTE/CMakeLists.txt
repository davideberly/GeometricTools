if(COMMAND cmake_policy)
    # Allow VERSION in the project() statement.
    cmake_policy(SET CMP0048 NEW)
endif()

set(GTE_VERSION_MAJOR 5)
set(GTE_VERSION_MINOR 12)

project(GTE VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR})

cmake_minimum_required(VERSION 3.8)
add_subdirectory(Applications)
add_subdirectory(Graphics)
add_subdirectory(Mathematics)
add_subdirectory(MathematicsGPU)
