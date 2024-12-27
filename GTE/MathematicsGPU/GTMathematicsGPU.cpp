// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.12.26

#include <MathematicsGPU/GTMathematicsGPUPCH.h>

// On Microsoft Windows 10/11, the preprocessor symbol GTE_USE_MSWINDOWS is
// added to the Visual Studio project configurations. If a project uses
// DirectX 11 (or later), the symbol GTE_USE_DIRECTX is added to the Visual
// Studio projects. If a project uses OpenGL, the symbol GTE_USE_OPENGL is
// added to the Visual Studio projects.
//
// On Linux, the make system has the defines GTE_USE_LINUX and GTE_USE_OPENGL.
//
// By default, GTE uses row-major order for storing matrices. Previously
// you had to add to the project settings the preprocessor symbol
// GTE_USE_ROW_MAJOR. The source code has been modified so that you no
// longer need to define GTE_USE_ROW_MAJOR. If you want column-major order
// instead, add to the project settings the preprocessor symbol
// GTE_USE_COL_MAJOR.
//
// By default, GTE multiplies a matrix M and a vector V using M*V, which is
// the vector-on-the-right convention. Previously you had to add to the
// project settings the preprocessor symbol GTE_USE_MAT_VEC. The source code
// has been modified so that you no longer need to define GTE_USE_MAT_VEC.
// If you want the multiplication to represent V*M instead, which is the
// vector-on-the-left convention, add to the project settings the preprocessor
// symbol GTE_USE_VEC_MAT.

#if defined(GTE_USE_MSWINDOWS)

// The _MSC_VER macro has integer values as described at the webpage
// https://learn.microsoft.com/en-us/cpp/overview/compiler-versions?view=msvc-170
//  Currently, projects are provided only for Visual Studio 2019/2022, but the
// macro allows you to compile with Visual Studio 2015 or later.
#if !defined(_MSC_VER)
#error Microsoft Visual Studio 2015 or later is required.
#endif
#if _MSC_VER < 1900
#error Microsoft Visual Studio 2015 or later is required.
#endif

#endif
