// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "RootFindingConsole.h"
#include <Applications/Timer.h>
#include <Mathematics/IEEEBinary.h>
#include <iostream>
#include <thread>

RootFindingConsole::RootFindingConsole(Parameters& parameters)
    :
    Console(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }
}

void RootFindingConsole::Execute()
{
    std::set<float> rootsCPU, rootsCPUMultithreaded, rootsGPU;

    Timer timer;
    double start, final, delta;
    std::string message;

    start = timer.GetSeconds();
    FindRootsCPU(rootsCPU);
    final = timer.GetSeconds();
    delta = final - start;
    message = "CPU: " + std::to_string(delta) + "\n";
    std::cout << message;

    start = timer.GetSeconds();
    FindRootsCPUMultithreaded(rootsCPUMultithreaded);
    final = timer.GetSeconds();
    delta = final - start;
    message = "CPU multithreaded: " + std::to_string(delta) + "\n";
    std::cout << message;

    start = timer.GetSeconds();
    FindRootsGPU(rootsGPU);
    final = timer.GetSeconds();
    delta = final - start;
    message = "GPU: " + std::to_string(delta) + "\n";
    std::cout << message;
}

bool RootFindingConsole::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Mathematics/RootFinding/Shaders/");

    if (mEnvironment.GetPath(mEngine->GetShaderName("RootFinder.cs")) == "")
    {
        LogError("Cannot find file " + mEngine->GetShaderName("RootFinder.cs"));
        return false;
    }

    return true;
}

float RootFindingConsole::MyFunction(float z)
{
    return (z - 1.1f) * (z + 2.2f);
}

void RootFindingConsole::FindRootsCPU(std::set<float>& roots)
{
    uint32_t const supTrailing = (1 << 23);
    for (uint32_t trailing = 0; trailing < supTrailing; ++trailing)
    {
        for (uint32_t biased = 0; biased < 255; ++biased)
        {
            IEEEBinary32 z0, z1;
            z0.encoding = (biased << 23) | trailing;
            z1.encoding = z0.encoding + 1;

            float f0 = MyFunction(z0.number);
            float f1 = MyFunction(z1.number);
            if (f0 * f1 <= 0.0f)
            {
                roots.insert(std::fabs(f0) <= std::fabs(f1) ? z0.number : z1.number);
            }

            z0.number = -z0.number;
            z1.number = -z1.number;
            f0 = MyFunction(z0.number);
            f1 = MyFunction(z1.number);
            if (f0 * f1 <= 0.0f)
            {
                roots.insert(std::fabs(f0) <= std::fabs(f1) ? z0.number : z1.number);
            }
        }
    }
}

void RootFindingConsole::FindRootsGPU(std::set<float>& roots)
{
    Environment env;
    std::string path = mEnvironment.GetPath(mEngine->GetShaderName("RootFinder.cs"));

    mProgramFactory->defines.Set("FUNCTION_BODY", "(z - 1.1f)*(z + 2.2f)");
    auto cprogram = mProgramFactory->CreateFromFile(path);
    LogAssert(cprogram != nullptr, "Faziled to compile program.");

    auto acBuffer = std::make_shared<StructuredBuffer>(1024, sizeof(Vector4<float>));
    acBuffer->MakeAppendConsume();
    acBuffer->SetCopy(Resource::Copy::STAGING_TO_CPU);
    acBuffer->SetNumActiveElements(0);

    cprogram->GetComputeShader()->Set("rootBounds", acBuffer);

    mEngine->Execute(cprogram, 512, 256, 1);

    mEngine->CopyGpuToCpu(acBuffer);
    uint32_t numActive = acBuffer->GetNumActiveElements();
    auto rootBounds = acBuffer->Get<Vector4<float>>();
    for (uint32_t i = 0; i < numActive; ++i)
    {
        auto const& rb = rootBounds[i];
        if (std::fabs(rb[1]) <= std::fabs(rb[3]))
        {
            roots.insert(rb[0]);
        }
        else
        {
            roots.insert(rb[2]);
        }
    }
}

void RootFindingConsole::FindSubRootsCPU(uint32_t tmin, uint32_t tsup, std::set<float>& roots)
{
    for (uint32_t trailing = tmin; trailing < tsup; ++trailing)
    {
        for (uint32_t biased = 0; biased < 255; ++biased)
        {
            IEEEBinary32 z0, z1;
            z0.encoding = (biased << 23) | trailing;
            z1.encoding = z0.encoding + 1;

            float f0 = MyFunction(z0.number);
            float f1 = MyFunction(z1.number);
            if (f0 * f1 <= 0.0f)
            {
                roots.insert(std::fabs(f0) <= std::fabs(f1) ? z0.number : z1.number);
            }

            z0.number = -z0.number;
            z1.number = -z1.number;
            f0 = MyFunction(z0.number);
            f1 = MyFunction(z1.number);
            if (f0 * f1 <= 0.0f)
            {
                roots.insert(std::fabs(f0) <= std::fabs(f1) ? z0.number : z1.number);
            }
        }
    }
}

void RootFindingConsole::FindRootsCPUMultithreaded(std::set<float>& roots)
{
    int32_t const numThreads = 16;
    uint32_t const supTrailing = (1 << 23);
    std::set<float> subRoots[numThreads];

    std::thread process[numThreads];
    for (int32_t t = 0; t < numThreads; ++t)
    {
        uint32_t tmin = t * supTrailing / numThreads;
        uint32_t tsup = (t + 1) * supTrailing / numThreads;
        auto rootFinder = std::bind(FindSubRootsCPU, tmin, tsup, std::ref(subRoots[t]));
        process[t] = std::thread([&rootFinder]() { rootFinder(); });
    }

    for (int32_t t = 0; t < numThreads; ++t)
    {
        process[t].join();
    }

    for (int32_t t = 0; t < numThreads; ++t)
    {
        for (auto const& z : subRoots[t])
        {
            roots.insert(z);
        }
    }
}
