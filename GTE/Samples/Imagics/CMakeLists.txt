if(COMMAND cmake_policy)
    # Allow VERSION in the project() statement.
    cmake_policy(SET CMP0048 NEW)
endif()

cmake_minimum_required(VERSION 3.8)

set(GTE_VERSION_MAJOR 7)
set(GTE_VERSION_MINOR 3)

project(Imagics VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR})

add_subdirectory(AdaptiveSkeletonClimbing2)
add_subdirectory(AdaptiveSkeletonClimbing3)
add_subdirectory(BSplineInterpolation)
add_subdirectory(Convolution)
add_subdirectory(ExtractLevelCurves)
add_subdirectory(ExtractLevelSurfaces)
add_subdirectory(ExtractRidges)
add_subdirectory(GaussianBlurring)
add_subdirectory(GpuGaussianBlur2)
add_subdirectory(GpuGaussianBlur3)
add_subdirectory(MedianFiltering)
add_subdirectory(SurfaceExtraction)
add_subdirectory(VideoStreams)
