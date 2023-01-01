// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/IntrTriangle2Triangle2.h>
using namespace gte;

class IntersectTriangles2DWindow2 : public Window2
{
public:
    IntersectTriangles2DWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;
    virtual bool OnMouseClick(int32_t button, int32_t state, int32_t x, int32_t y, uint32_t modifiers) override;
    virtual bool OnMouseMotion(int32_t button, int32_t x, int32_t y, uint32_t modifiers) override;

private:
    void DrawTriangle(std::array<Vector2<float>, 3> const& vertex, uint32_t colorL, uint32_t colorD);
    void DrawIntersection();
    void DoQuery();

    Triangle2<float> mTriangle[2];
    std::vector<Vector2<float>> mIntersection;
    TIQuery<float, Triangle2<float>, Triangle2<float>> mTIQuery;
    FIQuery<float, Triangle2<float>, Triangle2<float>> mFIQuery;
    int32_t mActive;
    bool mHasIntersection;
    bool mDoTIQuery;  // true, use mTIQuery; false, use mFIQuery
};
