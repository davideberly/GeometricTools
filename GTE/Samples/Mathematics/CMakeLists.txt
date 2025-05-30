if(COMMAND cmake_policy)
    # Allow VERSION in the project() statement.
    cmake_policy(SET CMP0048 NEW)
endif()

cmake_minimum_required(VERSION 3.8)

set(GTE_VERSION_MAJOR 7)
set(GTE_VERSION_MINOR 3)

project(Mathematics VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR})

add_subdirectory(ApproximateBezierCurveByArcs)
add_subdirectory(ApproximateEllipse2)
add_subdirectory(ApproximateEllipsesByArcs)
add_subdirectory(ApproximateEllipsoid3)
add_subdirectory(BSplineCurveFitter)
add_subdirectory(BSplineCurveReduction)
add_subdirectory(BSplineSurfaceFitter)
add_subdirectory(FitCone)
add_subdirectory(FitConeByEllipseAndPoints)
add_subdirectory(FitCylinder)
add_subdirectory(FitTorus)
add_subdirectory(GeodesicEllipsoid)
add_subdirectory(GeodesicHeightField)
add_subdirectory(Interpolation2D)
add_subdirectory(NURBSCircle)
add_subdirectory(NURBSCircularArc)
add_subdirectory(NURBSCurveExample)
add_subdirectory(NURBSSphere)
add_subdirectory(PartialSums)
add_subdirectory(PlaneEstimation)
add_subdirectory(RootFinding)
add_subdirectory(ShortestPath)
add_subdirectory(SymmetricEigensolver3x3)
add_subdirectory(ThinPlateSplines)
