// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GLSLVisualProgram.h>
using namespace gte;

GLSLVisualProgram::~GLSLVisualProgram()
{
    if (glIsProgram(mProgramHandle))
    {
        if (glIsShader(mVertexShaderHandle))
        {
            glDetachShader(mProgramHandle, mVertexShaderHandle);
            glDeleteShader(mVertexShaderHandle);
        }

        if (glIsShader(mPixelShaderHandle))
        {
            glDetachShader(mProgramHandle, mPixelShaderHandle);
            glDeleteShader(mPixelShaderHandle);
        }

        if (glIsShader(mGeometryShaderHandle))
        {
            glDetachShader(mProgramHandle, mGeometryShaderHandle);
            glDeleteShader(mGeometryShaderHandle);
        }

        glDeleteProgram(mProgramHandle);
    }
}

GLSLVisualProgram::GLSLVisualProgram(GLuint programHandle, GLuint vertexShaderHandle,
    GLuint pixedlShaderHandle, GLuint geometryShaderHandle)
    :
    mProgramHandle(programHandle),
    mVertexShaderHandle(vertexShaderHandle),
    mPixelShaderHandle(pixedlShaderHandle),
    mGeometryShaderHandle(geometryShaderHandle),
    mReflector(programHandle)
{
}

