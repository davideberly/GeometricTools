// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Console.h>
#include <string>
using namespace gte;

class ShaderReflectionConsole : public Console
{
public:
    ShaderReflectionConsole(Parameters& parameters);

    virtual void Execute() override;

private:
    bool SetEnvironment();
    void ReflectVertexColoring();
    void ReflectTexturing();
    void ReflectBillboards();
    void ReflectNestedStruct();
    void ReflectTextureArrays();
    void ReflectSimpleBuffers();
    void ReflectAppendConsume();

#if defined(GTE_USE_DIRECTX)
    uint32_t mCompileFlags;
#endif

    std::string mIOPath;
    std::string mExt;
};
