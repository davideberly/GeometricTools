// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.5.2022.12.12

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/IntrRay2SegmentMesh2.h>
#include <Mathematics/IntrSegment2SegmentMesh2.h>
using namespace gte;

// Expose this define to test the ray-mesh intersection query. Hide this
// define to test the segment-mesh intersection query.
#define GTE_DO_RAY_CAST

class IntersectRayPolygonWindow2 : public Window2
{
public:
    IntersectRayPolygonWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;
    virtual bool OnMouseMotion(int32_t button, int32_t x, int32_t y, uint32_t modifiers) override;

private:
    bool SetEnvironment();
    void CreatePolygon();

#if defined(GTE_DO_RAY_CAST)
    FIQuery<double, Ray2<double>, SegmentMesh2<double>> mRMQuery;
    void DoRayCast(int32_t x0, int32_t y0);
#else
    FIQuery<double, Segment2<double>, SegmentMesh2<double>> mSMQuery;
    void DoSegmentCast(int32_t x0, int32_t y0);
#endif

    static size_t constexpr maxDirections = 360;
    std::vector<std::array<int32_t, 4>> mDrawLineInputs;
    SegmentMesh2<double> mSegmentMesh;
};
