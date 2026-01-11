// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/NURBSCircle.h>
#include <Mathematics/SampleCircularArc.h>
using namespace gte;

class NURBSCircularArcWindow2 : public Window2
{
public:
    NURBSCircularArcWindow2(Parameters& parameters);

    virtual void OnIdle() override;
    virtual void OnDisplay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    SampleCircularArc<float> mSampler;
    Arc2<float> mArc;
    std::vector<Vector2<float>> mPoints;
    std::size_t mSelection;
};


