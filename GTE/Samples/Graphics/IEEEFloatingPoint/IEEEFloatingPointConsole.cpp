// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "IEEEFloatingPointConsole.h"
#include <Mathematics/IEEEBinary.h>

IEEEFloatingPointConsole::IEEEFloatingPointConsole(Parameters& parameters)
    :
    Console(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }
}

void IEEEFloatingPointConsole::Execute()
{
    std::string path = mEnvironment.GetPath(mEngine->GetShaderName("TestSubnormals.cs"));

    // With IEEE 754-2008 behavior that preserves subnormals, the output
    // fresult should have encoding 2 (number is 2^{-148}).  Instead
    // fresult.encoding = 0, which means that the GPU has flushed the
    // subnormal result to zero.
    IEEEBinary32 fresult;
    TestSubnormals<float, IEEEBinary32>(path, "float", fresult);

    // With IEEE 754-2008 behavior that preserves subnormals, the output
    // dresult should have encoding 2 (number is 2^{-1073}).  Indeed,
    // dresult.encoding = 2.
    IEEEBinary64 dresult;
    TestSubnormals<double, IEEEBinary64>(path, "double", dresult);
}

bool IEEEFloatingPointConsole::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/IEEEFloatingPoint/Shaders/");

    if (mEnvironment.GetPath(mEngine->GetShaderName("TestSubnormals.cs")) == "")
    {
        LogError("Cannot find file " + mEngine->GetShaderName("TestSubnormals.cs"));
        return false;
    }

    return true;
}
