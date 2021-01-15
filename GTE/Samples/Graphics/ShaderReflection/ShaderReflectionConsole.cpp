// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "ShaderReflectionConsole.h"

#if defined(GTE_USE_DIRECTX)
#include <Graphics/DX11/HLSLShaderFactory.h>
#endif

#if defined(GTE_USE_OPENGL)
#include <Graphics/GL45/GLSLVisualProgram.h>
#include <Graphics/GL45/GLSLComputeProgram.h>
#endif

ShaderReflectionConsole::ShaderReflectionConsole(Parameters& parameters)
    :
    Console(parameters)
{
#if defined(GTE_USE_DIRECTX)
    mCompileFlags =
        D3DCOMPILE_ENABLE_STRICTNESS |
        D3DCOMPILE_IEEE_STRICTNESS |
        D3DCOMPILE_PACK_MATRIX_ROW_MAJOR |
        D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
}

void ShaderReflectionConsole::Execute()
{
    ReflectVertexColoring();
    ReflectTexturing();
    ReflectBillboards();
    ReflectNestedStruct();
    ReflectTextureArrays();
    ReflectSimpleBuffers();
    ReflectAppendConsume();
}

void ShaderReflectionConsole::ReflectVertexColoring()
{
#if defined(GTE_USE_DIRECTX)
    HLSLReflection vshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/VertexColoring.vs.hlsl",
        "VSMain",
        "vs_5_0",
        ProgramDefines(),
        mCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout("Shaders/VertexColoring.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLReflection pshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/VertexColoring.ps.hlsl",
        "PSMain",
        "ps_5_0",
        ProgramDefines(),
        mCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout("Shaders/VertexColoring.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLVisualProgram>(mProgramFactory->CreateFromFiles(
        "Shaders/VertexColoring.vs.glsl", "Shaders/VertexColoring.ps.glsl", ""));
    if (program)
    {
        std::ofstream out("Shaders/VertexColoring.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
#endif
}

void ShaderReflectionConsole::ReflectTexturing()
{
#if defined(GTE_USE_DIRECTX)
    HLSLReflection vshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/Texturing.vs.hlsl",
        "VSMain",
        "vs_5_0",
        ProgramDefines(),
        mCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout("Shaders/Texturing.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLReflection pshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/Texturing.ps.hlsl",
        "PSMain",
        "ps_5_0",
        ProgramDefines(),
        mCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout("Shaders/Texturing.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLVisualProgram>(mProgramFactory->CreateFromFiles(
        "Shaders/Texturing.vs.glsl", "Shaders/Texturing.ps.glsl", ""));
    if (program)
    {
        std::ofstream out("Shaders/Texturing.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
#endif
}

void ShaderReflectionConsole::ReflectBillboards()
{
#if defined(GTE_USE_DIRECTX)
    HLSLReflection vshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/Billboards.vs.hlsl",
        "VSMain",
        "vs_5_0",
        ProgramDefines(),
        mCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout("Shaders/Billboards.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLReflection gshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/Billboards.gs.hlsl",
        "GSMain",
        "gs_5_0",
        ProgramDefines(),
        mCompileFlags);

    if (gshader.IsValid())
    {
        std::ofstream gsout("Shaders/Billboards.gsreflect.txt");
        if (gsout)
        {
            gshader.Print(gsout);
            gsout.close();
        }
    }

    HLSLReflection pshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/Billboards.ps.hlsl",
        "PSMain",
        "ps_5_0",
        ProgramDefines(),
        mCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout("Shaders/Billboards.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLVisualProgram>(mProgramFactory->CreateFromFiles(
        "Shaders/Billboards.vs.glsl", "Shaders/Billboards.ps.glsl", "Shaders/Billboards.gs.glsl"));
    if (program)
    {
        std::ofstream out("Shaders/Billboards.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
#endif
}

void ShaderReflectionConsole::ReflectNestedStruct()
{
#if defined(GTE_USE_DIRECTX)
    HLSLReflection cshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/NestedStruct.cs.hlsl",
        "CSMain",
        "cs_5_0",
        ProgramDefines(),
        mCompileFlags);

    if (cshader.IsValid())
    {
        std::ofstream csout("Shaders/NestedStruct.csreflect.txt");
        if (csout)
        {
            cshader.Print(csout);
            csout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLComputeProgram>(mProgramFactory->CreateFromFile(
        "Shaders/NestedStruct.cs.glsl"));
    if (program)
    {
        std::ofstream out("Shaders/NestedStruct.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
#endif
}

void ShaderReflectionConsole::ReflectTextureArrays()
{
#if defined(GTE_USE_DIRECTX)
    HLSLReflection vshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/TextureArrays.vs.hlsl",
        "VSMain",
        "vs_5_0",
        ProgramDefines(),
        mCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout("Shaders/TextureArrays.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLReflection pshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/TextureArrays.ps.hlsl",
        "PSMain",
        "ps_5_0",
        ProgramDefines(),
        mCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout("Shaders/TextureArrays.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLVisualProgram>(mProgramFactory->CreateFromFiles(
        "Shaders/TextureArrays.vs.glsl", "Shaders/TextureArrays.ps.glsl", ""));
    if (program)
    {
        std::ofstream out("Shaders/TextureArrays.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
#endif
}

void ShaderReflectionConsole::ReflectSimpleBuffers()
{
#if defined(GTE_USE_DIRECTX)
    HLSLReflection cshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/SimpleBuffers.cs.hlsl",
        "CSMain",
        "cs_5_0",
        ProgramDefines(),
        mCompileFlags);

    if (cshader.IsValid())
    {
        std::ofstream csout("Shaders/SimpleBuffers.csreflect.txt");
        if (csout)
        {
            cshader.Print(csout);
            csout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLComputeProgram>(mProgramFactory->CreateFromFile(
        "Shaders/SimpleBuffers.cs.glsl"));
    if (program)
    {
        std::ofstream out("Shaders/SimpleBuffers.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
#endif
}

void ShaderReflectionConsole::ReflectAppendConsume()
{
#if defined(GTE_USE_DIRECTX)
    HLSLReflection cshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/AppendConsume.cs.hlsl",
        "CSMain",
        "cs_5_0",
        ProgramDefines(),
        mCompileFlags);

    if (cshader.IsValid())
    {
        std::ofstream csout("Shaders/AppendConsume.csreflect.txt");
        if (csout)
        {
            cshader.Print(csout);
            csout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLComputeProgram>(mProgramFactory->CreateFromFile(
        "Shaders/AppendConsume.cs.glsl"));
    if (program)
    {
        std::ofstream out("Shaders/AppendConsume.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
#endif
}
