// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
using namespace gte;

class MedianFilteringWindow2 : public Window2
{
public:
    MedianFilteringWindow2(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    bool CreatePrograms(uint32_t txWidth, uint32_t txHeight);

    std::shared_ptr<Texture2> mOriginal;
    std::shared_ptr<Texture2> mImage[2];
    std::shared_ptr<OverlayEffect> mOverlay[2];

    // 0 = median 3x3 by insertion sort
    // 1 = median 3x3 by min-max
    // 2 = median 5x5 by insertion sort
    // 3 = median 5x5 by min-max
    uint32_t mSelection;
    std::shared_ptr<ComputeProgram> mMedianProgram[4];
    std::shared_ptr<ComputeProgram> mCProgram;
    uint32_t mNumXGroups, mNumYGroups;
    static std::string msName[4];
};
