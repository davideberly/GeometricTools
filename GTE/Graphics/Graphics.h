// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#if defined(GTE_USE_MSWINDOWS)

#if defined(GTE_USE_DIRECTX)
#include <Graphics/DX11/DX11Engine.h>
#include <Graphics/DX11/HLSLProgramFactory.h>
#endif

#if defined(GTE_USE_OPENGL)
#include <Graphics/GL46/WGL/WGLEngine.h>
#include <Graphics/GL46/GLSLProgramFactory.h>
#endif

#endif

#if defined(GTE_USE_LINUX)
#include <Graphics/GL46/GLX/GLXEngine.h>
#include <Graphics/GL46/GLSLProgramFactory.h>
#endif

