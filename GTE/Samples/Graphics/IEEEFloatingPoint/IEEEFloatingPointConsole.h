// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Console.h>
using namespace gte;

class IEEEFloatingPointConsole : public Console
{
public:
    IEEEFloatingPointConsole(Parameters& parameters);

    virtual void Execute() override;

private:
    bool SetEnvironment();

    template <typename Real, typename Binary>
    void TestSubnormals(std::string const& filename, std::string const& realname, Binary& result)
    {
        auto inputBuffer = std::make_shared<StructuredBuffer>(2, sizeof(Real));
        Real* input = inputBuffer->Get<Real>();
        Binary v0, v1;
        v0.encoding = 1;
        v1.encoding = 1;
        input[0] = v0.number;  // Smallest positive subnormal.
        input[1] = v1.number;  // Same as v0.

        // Compute v0+v1 and store in this buffer.
        auto outputBuffer = std::make_shared<StructuredBuffer>(1, sizeof(Real));
        outputBuffer->SetUsage(Resource::Usage::SHADER_OUTPUT);
        outputBuffer->SetCopy(Resource::Copy::STAGING_TO_CPU);
        Real* output = outputBuffer->Get<Real>();
        output[0] = (Real)0;

        mProgramFactory->defines.Set("REAL", realname);
        auto cprogram = mProgramFactory->CreateFromFile(filename);
        if (!cprogram)
        {
            LogError("Cannot load or compile cshader.");
        }
        auto const& cshader = cprogram->GetComputeShader();
        cshader->Set("inBuffer", inputBuffer);
        cshader->Set("outBuffer", outputBuffer);

        mEngine->Execute(cprogram, 1, 1, 1);
        mEngine->CopyGpuToCpu(outputBuffer);

        result.number = output[0];
    }
};
