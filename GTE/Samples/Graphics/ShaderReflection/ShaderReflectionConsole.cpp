// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

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
    Console(parameters),
#if defined(GTE_USE_DIRECTX)
    mCompileFlags(D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_IEEE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3),
#endif
    mIOPath(""),
    mExt("")
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }
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

bool ShaderReflectionConsole::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mIOPath = path + "/Samples/Graphics/ShaderReflection/Shaders/";
    mEnvironment.Insert(mIOPath);

#if defined(GTE_USE_DIRECTX)
    mExt = ".hlsl";
#else
    mExt = ".glsl";
#endif

    std::vector<std::string> inputs =
    {
        "AppendConsume.cs" + mExt,
        "Billboards.gs" + mExt,
        "Billboards.ps" + mExt,
        "Billboards.vs" + mExt,
        "NestedStruct.cs" + mExt,
        "SimpleBuffers.cs" + mExt,
        "TextureArrays.ps" + mExt,
        "TextureArrays.vs" + mExt,
        "Texturing.ps" + mExt,
        "Texturing.vs" + mExt,
        "VertexColoring.ps" + mExt,
        "VertexColoring.vs" + mExt
    };

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    return true;
}

void ShaderReflectionConsole::ReflectVertexColoring()
{
    std::string vsPath = mIOPath + "VertexColoring.vs" + mExt;
    std::string psPath = mIOPath + "VertexColoring.ps" + mExt;

#if defined(GTE_USE_DIRECTX)
    HLSLReflection vshader = HLSLShaderFactory::CreateFromFile(
        vsPath, "VSMain", "vs_5_0", ProgramDefines(), mCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout(mIOPath + "VertexColoring.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLReflection pshader = HLSLShaderFactory::CreateFromFile(
        psPath, "PSMain", "ps_5_0", ProgramDefines(), mCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout(mIOPath + "VertexColoring.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLVisualProgram>(
        mProgramFactory->CreateFromFiles(vsPath, psPath, ""));
    if (program)
    {
        std::ofstream out(mIOPath + "VertexColoring.glslreflect.txt");
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
    std::string vsPath = mIOPath + "Texturing.vs" + mExt;
    std::string psPath = mIOPath + "Texturing.ps" + mExt;

#if defined(GTE_USE_DIRECTX)
    HLSLReflection vshader = HLSLShaderFactory::CreateFromFile(
        vsPath, "VSMain", "vs_5_0", ProgramDefines(), mCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout(mIOPath + "Texturing.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLReflection pshader = HLSLShaderFactory::CreateFromFile(
        psPath, "PSMain", "ps_5_0", ProgramDefines(), mCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout(mIOPath + "Texturing.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLVisualProgram>(
        mProgramFactory->CreateFromFiles(vsPath, psPath, ""));
    if (program)
    {
        std::ofstream out(mIOPath + "Texturing.glslreflect.txt");
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
    std::string vsPath = mIOPath + "Billboards.vs" + mExt;
    std::string gsPath = mIOPath + "Billboards.gs" + mExt;
    std::string psPath = mIOPath + "Billboards.ps" + mExt;

#if defined(GTE_USE_DIRECTX)
    HLSLReflection vshader = HLSLShaderFactory::CreateFromFile(
        vsPath, "VSMain", "vs_5_0", ProgramDefines(), mCompileFlags);

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
        gsPath, "GSMain", "gs_5_0", ProgramDefines(), mCompileFlags);

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
        psPath, "PSMain", "ps_5_0", ProgramDefines(), mCompileFlags);

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
    auto program = std::dynamic_pointer_cast<GLSLVisualProgram>(
        mProgramFactory->CreateFromFiles(vsPath, psPath, gsPath));
    if (program)
    {
        std::ofstream out(mIOPath + "Billboards.glslreflect.txt");
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
    std::string csPath = mIOPath + "NestedStruct.cs" + mExt;

#if defined(GTE_USE_DIRECTX)
    HLSLReflection cshader = HLSLShaderFactory::CreateFromFile(
        csPath, "CSMain", "cs_5_0", ProgramDefines(), mCompileFlags);

    if (cshader.IsValid())
    {
        std::ofstream csout(mIOPath + "NestedStruct.csreflect.txt");
        if (csout)
        {
            cshader.Print(csout);
            csout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLComputeProgram>(
        mProgramFactory->CreateFromFile(csPath));
    if (program)
    {
        std::ofstream out(mIOPath + "NestedStruct.glslreflect.txt");
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
    std::string vsPath = mIOPath + "TextureArrays.vs" + mExt;
    std::string psPath = mIOPath + "TextureArrays.ps" + mExt;

#if defined(GTE_USE_DIRECTX)
    HLSLReflection vshader = HLSLShaderFactory::CreateFromFile(
        vsPath, "VSMain", "vs_5_0", ProgramDefines(), mCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout(mIOPath + "TextureArrays.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLReflection pshader = HLSLShaderFactory::CreateFromFile(
        psPath, "PSMain", "ps_5_0", ProgramDefines(), mCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout(mIOPath + "TextureArrays.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLVisualProgram>(
        mProgramFactory->CreateFromFiles(vsPath, psPath, ""));
    if (program)
    {
        std::ofstream out(mIOPath + "TextureArrays.glslreflect.txt");
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
    std::string csPath = mIOPath + "SimpleBuffers.cs" + mExt;

#if defined(GTE_USE_DIRECTX)
    HLSLReflection cshader = HLSLShaderFactory::CreateFromFile(
        csPath, "CSMain", "cs_5_0", ProgramDefines(), mCompileFlags);

    if (cshader.IsValid())
    {
        std::ofstream csout(mIOPath + "SimpleBuffers.csreflect.txt");
        if (csout)
        {
            cshader.Print(csout);
            csout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLComputeProgram>(
        mProgramFactory->CreateFromFile(csPath));
    if (program)
    {
        std::ofstream out(mIOPath + "SimpleBuffers.glslreflect.txt");
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
    std::string csPath = mIOPath + "AppendConsume.cs" + mExt;

#if defined(GTE_USE_DIRECTX)
    HLSLReflection cshader = HLSLShaderFactory::CreateFromFile(
        csPath, "CSMain", "cs_5_0", ProgramDefines(), mCompileFlags);

    if (cshader.IsValid())
    {
        std::ofstream csout(mIOPath + "AppendConsume.csreflect.txt");
        if (csout)
        {
            cshader.Print(csout);
            csout.close();
        }
    }
#endif

#if defined(GTE_USE_OPENGL)
    auto program = std::dynamic_pointer_cast<GLSLComputeProgram>(
        mProgramFactory->CreateFromFile(csPath));
    if (program)
    {
        std::ofstream out(mIOPath + "AppendConsume.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
#endif
}
