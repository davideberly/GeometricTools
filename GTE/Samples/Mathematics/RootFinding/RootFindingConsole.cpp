// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

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
    unsigned int const supTrailing = (1 << 23);
    for (unsigned int trailing = 0; trailing < supTrailing; ++trailing)
    {
        for (unsigned int biased = 0; biased < 255; ++biased)
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
    acBuffer->SetCopyType(Resource::COPY_STAGING_TO_CPU);
    acBuffer->SetNumActiveElements(0);

    cprogram->GetComputeShader()->Set("rootBounds", acBuffer);

    mEngine->Execute(cprogram, 512, 256, 1);

    mEngine->CopyGpuToCpu(acBuffer);
    unsigned int numActive = acBuffer->GetNumActiveElements();
    auto rootBounds = acBuffer->Get<Vector4<float>>();
    for (unsigned int i = 0; i < numActive; ++i)
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

void RootFindingConsole::FindSubRootsCPU(unsigned int tmin, unsigned int tsup, std::set<float>& roots)
{
    for (unsigned int trailing = tmin; trailing < tsup; ++trailing)
    {
        for (unsigned int biased = 0; biased < 255; ++biased)
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
    int const numThreads = 16;
    unsigned int const supTrailing = (1 << 23);
    std::set<float> subRoots[numThreads];

    std::thread process[numThreads];
    for (int t = 0; t < numThreads; ++t)
    {
        unsigned int tmin = t * supTrailing / numThreads;
        unsigned int tsup = (t + 1) * supTrailing / numThreads;
        auto rootFinder = std::bind(FindSubRootsCPU, tmin, tsup, std::ref(subRoots[t]));
        process[t] = std::thread([&rootFinder]() { rootFinder(); });
    }

    for (int t = 0; t < numThreads; ++t)
    {
        process[t].join();
    }

    for (int t = 0; t < numThreads; ++t)
    {
        for (auto const& z : subRoots[t])
        {
            roots.insert(z);
        }
    }
}
