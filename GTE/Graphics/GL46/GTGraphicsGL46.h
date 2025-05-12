// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

// GL46/Engine
#include <Graphics/GL46/GL46.h>
#include <Graphics/GL46/GL46Engine.h>
#include <Graphics/GL46/GL46GraphicsObject.h>

// GL46/Engine/InputLayout
#include <Graphics/GL46/GL46InputLayout.h>
#include <Graphics/GL46/GL46InputLayoutManager.h>

// GL46/Engine/Resources
#include <Graphics/GL46/GL46Resource.h>

// GL46/Engine/Resources/Buffers
#include <Graphics/GL46/GL46AtomicCounterBuffer.h>
#include <Graphics/GL46/GL46Buffer.h>
#include <Graphics/GL46/GL46ConstantBuffer.h>
#include <Graphics/GL46/GL46IndexBuffer.h>
#include <Graphics/GL46/GL46StructuredBuffer.h>
#include <Graphics/GL46/GL46VertexBuffer.h>

// GL46/Engine/Resources/Textures
#include <Graphics/GL46/GL46DrawTarget.h>
#include <Graphics/GL46/GL46Texture.h>
#include <Graphics/GL46/GL46Texture1.h>
#include <Graphics/GL46/GL46Texture1Array.h>
#include <Graphics/GL46/GL46Texture2.h>
#include <Graphics/GL46/GL46Texture2Array.h>
#include <Graphics/GL46/GL46Texture3.h>
#include <Graphics/GL46/GL46TextureArray.h>
#include <Graphics/GL46/GL46TextureCube.h>
#include <Graphics/GL46/GL46TextureCubeArray.h>
#include <Graphics/GL46/GL46TextureDS.h>
#include <Graphics/GL46/GL46TextureRT.h>
#include <Graphics/GL46/GL46TextureSingle.h>

// GL46/Engine/State
#include <Graphics/GL46/GL46BlendState.h>
#include <Graphics/GL46/GL46DrawingState.h>
#include <Graphics/GL46/GL46DepthStencilState.h>
#include <Graphics/GL46/GL46RasterizerState.h>
#include <Graphics/GL46/GL46SamplerState.h>

// GL46/GLSL
#include <Graphics/GL46/GLSLComputeProgram.h>
#include <Graphics/GL46/GLSLProgramFactory.h>
#include <Graphics/GL46/GLSLReflection.h>
#include <Graphics/GL46/GLSLShader.h>
#include <Graphics/GL46/GLSLVisualProgram.h>

#if defined(GTE_USE_MSWINDOWS)
#include <Graphics/GL46/WGL/WGLEngine.h>
#endif

#if defined(GTE_USE_LINUX)
#include <Graphics/GL46/GLX/GLXEngine.h>
#endif

