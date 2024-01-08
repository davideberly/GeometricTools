if(COMMAND cmake_policy)
    # Allow VERSION in the project() statement.
    cmake_policy(SET CMP0048 NEW)
endif()

cmake_minimum_required(VERSION 3.8)

set(GTE_VERSION_MAJOR 7)
set(GTE_VERSION_MINOR 0)

project(SceneGraph VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR})

add_subdirectory(BillboardNodes)
add_subdirectory(BlendedAnimations)
add_subdirectory(BspNodes)
add_subdirectory(CameraAndLightNodes)
add_subdirectory(Castle)
add_subdirectory(CLODMeshes)
add_subdirectory(DLODNodes)
add_subdirectory(IKControllers)
add_subdirectory(MorphControllers)
add_subdirectory(MorphFaces)
add_subdirectory(ParticleControllers)
add_subdirectory(Picking)
add_subdirectory(PointControllers)
add_subdirectory(SwitchNodes)
add_subdirectory(Terrain)
