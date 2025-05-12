// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GLSLComputeProgram.h>
using namespace gte;

GLSLComputeProgram::~GLSLComputeProgram()
{
    if (glIsProgram(mProgramHandle))
    {
        if (glIsShader(mComputeShaderHandle))
        {
            glDetachShader(mProgramHandle, mComputeShaderHandle);
            glDeleteShader(mComputeShaderHandle);
        }

        glDeleteProgram(mProgramHandle);
    }
}

GLSLComputeProgram::GLSLComputeProgram(GLuint programHandle, GLuint computeShaderHandle)
    :
    mProgramHandle(programHandle),
    mComputeShaderHandle(computeShaderHandle),
    mReflector(programHandle)
{
}

