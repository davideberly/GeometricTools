if(COMMAND cmake_policy)
    # Allow VERSION in the project() statement.
    cmake_policy(SET CMP0048 NEW)
endif()

set(GTE_VERSION_MAJOR 8)
set(GTE_VERSION_MINOR 2)

cmake_minimum_required(VERSION 3.10)

project(Geometrics VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR})

add_subdirectory(CLODPolyline)
add_subdirectory(ConformalMapping)
add_subdirectory(ConstrainedDelaunay2D)
add_subdirectory(ConvexHull2D)
add_subdirectory(ConvexHull3D)
add_subdirectory(ConvexHullSimplePolygon)
add_subdirectory(Delaunay2D)
add_subdirectory(Delaunay3D)
add_subdirectory(DisjointIntervalsRectangles)
add_subdirectory(ExtremalQuery)
add_subdirectory(GenerateMeshUVs)
add_subdirectory(IncrementalDelaunay2)
add_subdirectory(MinimalCycleBasis)
add_subdirectory(MinimumAreaBox2D)
add_subdirectory(MinimumAreaCircle2D)
add_subdirectory(MinimumVolumeBox3D)
add_subdirectory(MinimumVolumeSphere3D)
add_subdirectory(PolygonBooleanOperations)
add_subdirectory(PolylineOffset)
add_subdirectory(SplitMeshByPlane)
add_subdirectory(TriangulationCDT)
add_subdirectory(TriangulationEC)
add_subdirectory(VertexCollapseMesh)

add_custom_target(Samples_Geometrics)
add_dependencies(Samples_Geometrics
    CLODPolyline
    ConformalMapping
    ConstrainedDelaunay2D
    ConvexHull2D
    ConvexHull3D
    ConvexHullSimplePolygon
    Delaunay2D
    Delaunay3D
    DisjointIntervalsRectangles
    ExtremalQuery
    GenerateMeshUVs
    IncrementalDelaunay2
    MinimalCycleBasis
    MinimumAreaBox2D
    MinimumAreaCircle2D
    MinimumVolumeBox3D
    MinimumVolumeSphere3D
    PolygonBooleanOperations
    PolylineOffset
    SplitMeshByPlane
    TriangulationCDT
    TriangulationEC
    VertexCollapseMesh
)
