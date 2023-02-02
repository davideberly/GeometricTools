// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/IncrementalDelaunay2.h>
using namespace gte;

class IncrementalDelaunay2Window2 : public Window2
{
public:
    IncrementalDelaunay2Window2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual void DrawScreenOverlay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;
    virtual bool OnMouseClick(int32_t button, int32_t state, int32_t x, int32_t y, uint32_t modifiers) override;

private:
    float mSize;
    IncrementalDelaunay2<float> mDelaunay;
    std::vector<Vector2<float>> mInputs;
    std::vector<Vector2<float>> mVertices;
    std::vector<std::array<size_t, 3>> mTriangles;
    IncrementalDelaunay2<float>::SearchInfo mInfo;
    size_t mContainingTriangle;
    std::string mMessage;
};
