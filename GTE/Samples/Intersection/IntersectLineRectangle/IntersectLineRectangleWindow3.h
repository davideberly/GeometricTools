// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.12.02

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/IntrLine3Rectangle3.h>
#include <Mathematics/IntrRay3Rectangle3.h>
#include <Mathematics/IntrSegment3Rectangle3.h>
using namespace gte;

// Expose only one of these at a time.
#define USE_LINE_RECTANGLE_QUERY
//#define USE_RAY_RECTANGLE_QUERY
//#define USE_SEGMENT_RECTANGLE_QUERY

class IntersectLineRectangleWindow3 : public Window3
{
public:
    IntersectLineRectangleWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();
    void Translate(int32_t direction, float delta);
    void Rotate(int32_t direction, float delta);
    void DoIntersectionQuery();

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<Visual> mLinearMesh;
    std::shared_ptr<Visual> mRectangleMesh;
    std::shared_ptr<Visual> mSphereMesh;

    Rectangle3<float> mRectangle;

#if defined(USE_LINE_RECTANGLE_QUERY)
    Line3<float> mLine;
    FIQuery<float, Line3<float>, Rectangle3<float>> mQuery;
    FIQuery<float, Line3<float>, Rectangle3<float>>::Result mResult;
#endif

#if defined(USE_RAY_RECTANGLE_QUERY)
    Ray3<float> mRay;
    FIQuery<float, Ray3<float>, Rectangle3<float>> mQuery;
    FIQuery<float, Ray3<float>, Rectangle3<float>>::Result mResult;
#endif

#if defined(USE_SEGMENT_RECTANGLE_QUERY)
    Segment3<float> mSegment;
    FIQuery<float, Segment3<float>, Rectangle3<float>> mQuery;
    FIQuery<float, Segment3<float>, Rectangle3<float>>::Result mResult;
#endif
};
