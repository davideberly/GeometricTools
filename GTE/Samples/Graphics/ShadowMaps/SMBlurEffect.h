// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

#pragma once

#include <Graphics/GraphicsEngine.h>
#include <Graphics/ComputeProgram.h>
#include <Graphics/ProgramFactory.h>
using namespace gte;

class SMBlurEffect
{
public:
    SMBlurEffect(std::shared_ptr<ProgramFactory> const& factory,
        std::string const& csPath, uint32_t numXThreads, uint32_t numYThreads,
        uint32_t numXGroups, uint32_t numYGroups);

    void SetInputImage(std::shared_ptr<Texture2> const& input);
    void SetOutputImage(std::shared_ptr<Texture2> const& output);
    void Execute(std::shared_ptr<GraphicsEngine> const& engine);

private:
    uint32_t mNumXThreads, mNumYThreads;
    uint32_t mNumXGroups, mNumYGroups;
    std::shared_ptr<ComputeProgram> mProgram;
};
