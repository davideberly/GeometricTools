// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

#include "SMBlurEffect.h"

SMBlurEffect::SMBlurEffect(std::shared_ptr<ProgramFactory> const& factory,
    std::string const& csPath, uint32_t numXThreads, uint32_t numYThreads,
    uint32_t numXGroups, uint32_t numYGroups)
    :
    mNumXThreads(numXThreads),
    mNumYThreads(numYThreads),
    mNumXGroups(numXGroups),
    mNumYGroups(numYGroups),
    mProgram{}
{
    factory->defines.Set("NUM_X_THREADS", mNumXThreads);
    factory->defines.Set("NUM_Y_THREADS", mNumYThreads);
    mProgram = factory->CreateFromFile(csPath);
    factory->defines.Clear();

    LogAssert(mProgram, "Failed to compile " + csPath);
}

void SMBlurEffect::SetInputImage(std::shared_ptr<Texture2> const& input)
{
    auto const& cshader = mProgram->GetComputeShader();
    cshader->Set("inImage", input);
}

void SMBlurEffect::SetOutputImage(std::shared_ptr<Texture2> const& output)
{
    auto const& cshader = mProgram->GetComputeShader();
    cshader->Set("outImage", output);
}

void SMBlurEffect::Execute(std::shared_ptr<GraphicsEngine> const& engine)
{
    engine->Execute(mProgram, mNumXGroups, mNumYGroups, 1);
}
