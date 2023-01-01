// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
using namespace gte;

//#define SAVE_RENDERING_TO_DISK

class GaussianBlurringWindow2 : public Window2
{
public:
    GaussianBlurringWindow2(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    bool CreateImages();
    bool CreateShader();

    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Texture2> mImage[2];
    std::shared_ptr<ComputeProgram> mGaussianBlurProgram;
    uint32_t mNumXThreads, mNumYThreads;
    uint32_t mNumXGroups, mNumYGroups;
    int32_t mPass;

#if defined(SAVE_RENDERING_TO_DISK)
    std::shared_ptr<DrawTarget> mTarget;
#endif
};
