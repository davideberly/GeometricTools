// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "AppendConsumeBuffersConsole.h"

AppendConsumeBuffersConsole::AppendConsumeBuffersConsole(Parameters& parameters)
    :
    Console(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }
}

void AppendConsumeBuffersConsole::Execute()
{
    auto csPath = mEnvironment.GetPath(mEngine->GetShaderName("AppendConsume.cs"));
    auto program = mProgramFactory->CreateFromFile(csPath);
    if (!program)
    {
        // The program factory will generate Log* messages.
        return;
    }

    // Create 32 particles, stored in currentState to be "consumed".
    struct Particle
    {
        int32_t location[2];
    };
    int32_t const numInputs = 32;
    auto currentState = std::make_shared<StructuredBuffer>(numInputs, sizeof(Particle));
    currentState->MakeAppendConsume();

    Particle* particle = currentState->Get<Particle>();
    for (int32_t i = 0; i < numInputs; ++i)
    {
        particle[i].location[0] = i;
        particle[i].location[1] = rand();
    }

    // The next set of particles is created from the initial set.
    auto nextState = std::make_shared<StructuredBuffer>(numInputs, sizeof(Particle));
    nextState->MakeAppendConsume();
    nextState->SetCopy(Resource::Copy::STAGING_TO_CPU);

    // Start with an empty buffer to which particles are "appended".
    nextState->SetNumActiveElements(0);

    auto const& cshader = program->GetComputeShader();
    cshader->Set("currentState", currentState);
    cshader->Set("nextState", nextState);

    // Compute the next set of particles.
    mEngine->Execute(program, 1, 1, 1);

    // Read back the data from the GPU to test whether we really have
    // consumed half the initial set.
    mEngine->CopyGpuToCpu(nextState);
    int32_t numNextState = nextState->GetNumActiveElements();
    LogAssert(numNextState == numInputs / 2, "Invalid number of active elements.");

    // Verify that the data was consumed properly.
    Particle* nextParticle = nextState->Get<Particle>();
    for (int32_t i = 0; i < numInputs / 2; ++i)
    {
        int32_t j = nextParticle[i].location[0];
        LogAssert((j & 1) == 0, "Invalid index.");
        LogAssert(nextParticle[i].location[1] == particle[j].location[1], "Invalid value.");
    }
}

bool AppendConsumeBuffersConsole::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/AppendConsumeBuffers/Shaders/");

    if (mEnvironment.GetPath(mEngine->GetShaderName("AppendConsume.cs")) == "")
    {
        LogError("Cannot find file " + mEngine->GetShaderName("AppendConsume.cs"));
        return false;
    }

    return true;
}
