cmake_minimum_required(VERSION 3.8)

project(gtgraphics VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR})

option(BUILD_RELEASE_LIB "Build release library" OFF)
option(BUILD_SHARED_LIB "Build shared library" OFF)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-DGTE_USE_LINUX -DGTE_USE_ROW_MAJOR -DGTE_USE_MAT_VEC -DGTE_USE_OPENGL -DGTE_DISABLE_PCH)
add_compile_options(-c -Wall -Werror)
if(BUILD_RELEASE_LIB)
    add_compile_definitions(NDEBUG)
    add_compile_options(-O3)
else()
    add_compile_definitions(_DEBUG)
    add_compile_options(-g)
endif()

set(GTE_ROOT ${PROJECT_SOURCE_DIR}/..)
set(GTE_INCLUDE_DIR ${GTE_ROOT})
set(GTE_LIB_PREFIX ${GTE_ROOT}/lib/${CMAKE_BUILD_TYPE})

include_directories(${GTE_INCLUDE_DIR})

set(GTE_CPP_FILES
AmbientLightEffect.cpp
AreaLightEffect.cpp
BaseEngine.cpp
BillboardNode.cpp
BlendState.cpp
BlendTransformController.cpp
BspNode.cpp
Buffer.cpp
BumpMapEffect.cpp
Camera.cpp
CLODMesh.cpp
CollisionMesh.cpp
ConstantBuffer.cpp
ConstantColorEffect.cpp
ControlledObject.cpp
Controller.cpp
CubeMapEffect.cpp
Culler.cpp
DataFormat.cpp
DepthStencilState.cpp
DirectionalLightEffect.cpp
DirectionalLightTextureEffect.cpp
DLODNode.cpp
DrawingState.cpp
DrawTarget.cpp
Font.cpp
FontArialW400H12.cpp
FontArialW400H14.cpp
FontArialW400H16.cpp
FontArialW400H18.cpp
FontArialW700H12.cpp
FontArialW700H14.cpp
FontArialW700H16.cpp
FontArialW700H18.cpp
GEDrawTarget.cpp
GEObject.cpp
GlossMapEffect.cpp
GraphicsEngine.cpp
GraphicsObject.cpp
GTGraphics.cpp
IKController.cpp
IndexBuffer.cpp
IndirectArgumentsBuffer.cpp
KeyframeController.cpp
Light.cpp
LightCameraGeometry.cpp
LightEffect.cpp
Lighting.cpp
Material.cpp
MeshFactory.cpp
MorphController.cpp
Node.cpp
OverlayEffect.cpp
ParticleController.cpp
Particles.cpp
Picker.cpp
PickRecord.cpp
PlanarReflectionEffect.cpp
PlanarShadowEffect.cpp
PointController.cpp
PointLightEffect.cpp
PointLightTextureEffect.cpp
ProgramDefines.cpp
ProgramFactory.cpp
ProjectedTextureEffect.cpp
PVWUpdater.cpp
RasterizerState.cpp
RawBuffer.cpp
Resource.cpp
SamplerState.cpp
Shader.cpp
SkinController.cpp
Spatial.cpp
SphereMapEffect.cpp
SpotLightEffect.cpp
StructuredBuffer.cpp
SwitchNode.cpp
Terrain.cpp
TextEffect.cpp
Texture.cpp
Texture1.cpp
Texture1Array.cpp
Texture2.cpp
Texture2Array.cpp
Texture2Effect.cpp
Texture3.cpp
Texture3Effect.cpp
TextureArray.cpp
TextureBuffer.cpp
TextureCube.cpp
TextureCubeArray.cpp
TextureDS.cpp
TextureRT.cpp
TextureSingle.cpp
TransformController.cpp
TypedBuffer.cpp
VertexBuffer.cpp
VertexColorEffect.cpp
VertexFormat.cpp
ViewVolume.cpp
ViewVolumeNode.cpp
Visual.cpp
VisualEffect.cpp
VolumeFogEffect.cpp
GL46/GL46.cpp
GL46/GL46AtomicCounterBuffer.cpp
GL46/GL46BlendState.cpp
GL46/GL46Buffer.cpp
GL46/GL46ConstantBuffer.cpp
GL46/GL46DepthStencilState.cpp
GL46/GL46DrawingState.cpp
GL46/GL46DrawTarget.cpp
GL46/GL46Engine.cpp
GL46/GL46GraphicsObject.cpp
GL46/GL46IndexBuffer.cpp
GL46/GL46InputLayout.cpp
GL46/GL46InputLayoutManager.cpp
GL46/GL46RasterizerState.cpp
GL46/GL46Resource.cpp
GL46/GL46SamplerState.cpp
GL46/GL46StructuredBuffer.cpp
GL46/GL46Texture.cpp
GL46/GL46Texture1.cpp
GL46/GL46Texture1Array.cpp
GL46/GL46Texture2.cpp
GL46/GL46Texture2Array.cpp
GL46/GL46Texture3.cpp
GL46/GL46TextureArray.cpp
GL46/GL46TextureCube.cpp
GL46/GL46TextureCubeArray.cpp
GL46/GL46TextureDS.cpp
GL46/GL46TextureRT.cpp
GL46/GL46TextureSingle.cpp
GL46/GL46VertexBuffer.cpp
GL46/GLSLComputeProgram.cpp
GL46/GLSLProgramFactory.cpp
GL46/GLSLReflection.cpp
GL46/GLSLShader.cpp
GL46/GLSLVisualProgram.cpp
GL46/GTGraphicsGL46.cpp
GL46/GLX/GLXEngine.cpp
GL46/GLX/GLXExtensions.cpp)

if(BUILD_SHARED_LIB)
    add_library(${PROJECT_NAME} SHARED ${GTE_CPP_FILES})
    set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR} VERSION ${PROJECT_VERSION})
    set(GTE_LIB_DIR ${GTE_LIB_PREFIX}Shared)
else()
    add_library(${PROJECT_NAME} STATIC ${GTE_CPP_FILES})
    set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
    set(GTE_LIB_DIR ${GTE_LIB_PREFIX}Static)
endif()
set(LIBRARY_OUTPUT_PATH ${GTE_LIB_DIR} CACHE PATH "Library directory" FORCE)
SET(LIBRARY_OUTPUT_PATH ${GTE_LIB_DIR})
