if(COMMAND cmake_policy)
    # Allow VERSION in the project() statement.
    cmake_policy(SET CMP0048 NEW)
endif()

set(GTE_VERSION_MAJOR 6)
set(GTE_VERSION_MINOR 8)

project(Distance VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR})

cmake_minimum_required(VERSION 3.8)
add_subdirectory(DistanceAlignedBoxes)
add_subdirectory(DistanceAlignedBoxOrientedBox)
add_subdirectory(DistanceLine2Box2)
add_subdirectory(DistanceOrientedBoxConeFrustum)
add_subdirectory(DistanceOrientedBoxes)
add_subdirectory(DistancePointConvexPolyhedron)
add_subdirectory(DistancePointHyperellipsoid)
add_subdirectory(DistanceRectangleBox)
add_subdirectory(DistanceSegments3)
add_subdirectory(DistanceTriangleBox)
