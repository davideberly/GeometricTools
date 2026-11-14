cmake_minimum_required(VERSION 3.30...4.4)

project(gtgraphics
	VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR}
	DESCRIPTION "GTE Graphics engine library"
	LANGUAGES CXX
)

set(gtgraphics_CPP_FILES
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
)
set(gtgraphics_BACKEND_CPP_FILES "")
set(gtgraphics_DX11_SHADER_INCLUDES "")
if(GTE_USE_DIRECTX)
	list(APPEND gtgraphics_BACKEND_CPP_FILES
		DX11/DX11.cpp
		DX11/DX11BlendState.cpp
		DX11/DX11Buffer.cpp
		DX11/DX11ComputeShader.cpp
		DX11/DX11ConstantBuffer.cpp
		DX11/DX11DepthStencilState.cpp
		DX11/DX11DrawingState.cpp
		DX11/DX11DrawTarget.cpp
		DX11/DX11Engine.cpp
		DX11/DX11GeometryShader.cpp
		DX11/DX11GraphicsObject.cpp
		DX11/DX11IndexBuffer.cpp
		DX11/DX11IndirectArgumentsBuffer.cpp
		DX11/DX11InputLayout.cpp
		DX11/DX11InputLayoutManager.cpp
		DX11/DX11PerformanceCounter.cpp
		DX11/DX11PixelShader.cpp
		DX11/DX11RasterizerState.cpp
		DX11/DX11RawBuffer.cpp
		DX11/DX11Resource.cpp
		DX11/DX11SamplerState.cpp
		DX11/DX11Shader.cpp
		DX11/DX11StructuredBuffer.cpp
		DX11/DX11Texture.cpp
		DX11/DX11Texture1.cpp
		DX11/DX11Texture1Array.cpp
		DX11/DX11Texture2.cpp
		DX11/DX11Texture2Array.cpp
		DX11/DX11Texture3.cpp
		DX11/DX11TextureArray.cpp
		DX11/DX11TextureBuffer.cpp
		DX11/DX11TextureCube.cpp
		DX11/DX11TextureCubeArray.cpp
		DX11/DX11TextureDS.cpp
		DX11/DX11TextureRT.cpp
		DX11/DX11TextureSingle.cpp
		DX11/DX11VertexBuffer.cpp
		DX11/DX11VertexShader.cpp
		DX11/DXGIAdapter.cpp
		DX11/DXGIOutput.cpp
		DX11/GTGraphicsDX11.cpp
		DX11/HLSLBaseBuffer.cpp
		DX11/HLSLByteAddressBuffer.cpp
		DX11/HLSLConstantBuffer.cpp
		DX11/HLSLParameter.cpp
		DX11/HLSLProgramFactory.cpp
		DX11/HLSLReflection.cpp
		DX11/HLSLResource.cpp
		DX11/HLSLResourceBindInfo.cpp
		DX11/HLSLSamplerState.cpp
		DX11/HLSLShader.cpp
		DX11/HLSLShaderFactory.cpp
		DX11/HLSLShaderType.cpp
		DX11/HLSLShaderVariable.cpp
		DX11/HLSLStructuredBuffer.cpp
		DX11/HLSLTexture.cpp
		DX11/HLSLTextureArray.cpp
		DX11/HLSLTextureBuffer.cpp
	)
	list(APPEND gtgraphics_DX11_SHADER_INCLUDES
		DX11/FloatFunction.hlsli
		DX11/SqrtEstimate.hlsli
	)
	set_source_files_properties(${gtgraphics_DX11_SHADER_INCLUDES}
		PROPERTIES HEADER_FILE_ONLY TRUE
	)

elseif(GTE_USE_OPENGL)
	list(APPEND gtgraphics_BACKEND_CPP_FILES
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
	)
	if(WIN32)
		list(APPEND gtgraphics_BACKEND_CPP_FILES
			GL46/WGL/WGLEngine.cpp
			GL46/WGL/WGLExtensions.cpp
		)
	else()
		list(APPEND gtgraphics_BACKEND_CPP_FILES
			GL46/GLX/GLXEngine.cpp
			GL46/GLX/GLXExtensions.cpp
		)
	endif()
endif()

if(GTE_BUILD_SHARED_LIBS)
	add_library(${PROJECT_NAME} SHARED)
	set_target_properties(${PROJECT_NAME}
		PROPERTIES
			SOVERSION ${PROJECT_VERSION_MAJOR}
			VERSION ${PROJECT_VERSION}
	)
else()
	add_library(${PROJECT_NAME} STATIC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
endif()

target_sources(${PROJECT_NAME}
	PRIVATE
		${gtgraphics_CPP_FILES}
		${gtgraphics_BACKEND_CPP_FILES}
		${gtgraphics_DX11_SHADER_INCLUDES}
)

file(GLOB_RECURSE gtgraphics_HEADER_FILES
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)
target_sources(${PROJECT_NAME}
	INTERFACE
	FILE_SET fs_gtgraphics_headers
	TYPE
	HEADERS
	BASE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/..
	FILES
	${gtgraphics_HEADER_FILES}
)

add_library(GTE::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(${PROJECT_NAME} PUBLIC gte_usage_requirements)
target_link_libraries(${PROJECT_NAME} PRIVATE gte_warning_levels)

target_link_libraries(${PROJECT_NAME} PUBLIC GTE::gtmathematics)
target_link_libraries(${PROJECT_NAME} PRIVATE ${GTE_GRAPHICS_BACKEND_LIBRARIES})