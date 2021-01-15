// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "PartialSumsConsole.h"
#include <random>
#include <iomanip>

PartialSumsConsole::PartialSumsConsole(Parameters& parameters)
    :
    Console(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }
}

void PartialSumsConsole::Execute()
{
    std::string path = mEnvironment.GetPath(mEngine->GetShaderName("PartialSums.cs"));

    // Compute partial sums of 8 numbers.
    int const LOGN = 3;
    int const n = (1 << LOGN);

    // Use a Mersenne twister engine for random numbers.
    std::mt19937 mte;
    std::uniform_real_distribution<float> urd(0.0f, 1.0f);

    // Select random numbers and store as the diagonal of an n-by-n texture.
    auto sum = std::make_shared<Texture2>(DF_R32_FLOAT, n, n);
    sum->SetUsage(Resource::SHADER_OUTPUT);
    sum->SetCopyType(Resource::COPY_STAGING_TO_CPU);
    auto data = sum->Get<float>();
    std::memset(data, 0, sum->GetNumBytes());
    for (int i = 0; i < n; ++i)
    {
        data[i + n*i] = urd(mte);
    }

    // Create the shader for each p with 1 <= p <= log(n).
    std::shared_ptr<ComputeProgram> partialSumProgram[LOGN];
    for (int i = 0, p = 1; i < LOGN; ++i, ++p)
    {
        mProgramFactory->defines.Set("NUM_X_THREADS", (1 << (LOGN - p)));
        mProgramFactory->defines.Set("NUM_Y_THREADS", (1 << i));
        mProgramFactory->defines.Set("TWO_P", (1 << p));
        mProgramFactory->defines.Set("TWO_PM1", (1 << i));
        partialSumProgram[i] = mProgramFactory->CreateFromFile(path);
        LogAssert(partialSumProgram[i] != nullptr, "Failed to compile program " + std::to_string(i));
        partialSumProgram[i]->GetComputeShader()->Set("sum", sum);
    }

    // Execute the shaders.
    for (int i = 0; i < LOGN; ++i)
    {
        mEngine->Execute(partialSumProgram[i], 1, 1, 1);
    }

    // Read back the results from GPU memory.
    mEngine->CopyGpuToCpu(sum);

    // Write the entire 2D sum texture to a file.  The first column contains
    // the partial sums.  The other nonzero entries in the texture are the
    // intermediate values computed by the shaders.
    std::ofstream output("PartialSumsResult.txt");
    output << std::setprecision(6) << std::left << std::setfill('0');
    for (int row = 0; row < n; ++row)
    {
        for (int col = 0; col < n; ++col)
        {
            output << std::setw(8) << data[col + n*row] << ' ';
        }
        output << std::endl;
    }
    output << std::endl;
    output.close();
}

bool PartialSumsConsole::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Mathematics/PartialSums/Shaders/");

    if (mEnvironment.GetPath(mEngine->GetShaderName("PartialSums.cs")) == "")
    {
        LogError("Cannot find file " + mEngine->GetShaderName("PartialSums.cs"));
        return false;
    }

    return true;
}
