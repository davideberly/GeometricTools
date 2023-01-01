// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/DistLine2AlignedBox2.h>
#include <Mathematics/DistLine2OrientedBox2.h>
#include <Mathematics/DistRay2AlignedBox2.h>
#include <Mathematics/DistRay2OrientedBox2.h>
#include <Mathematics/DistSegment2AlignedBox2.h>
#include <Mathematics/DistSegment2OrientedBox2.h>
using namespace gte;

// Expose only one of these defines.
#define USE_QUERY_LINE
//#define USE_QUERY_RAY
//#define USE_QUERY_SEGMENT

// Comment out this define for a line-oriented_box query.
#define USE_QUERY_ALIGNED_BOX

class DistanceLine2Box2Window2 : public Window2
{
public:
    DistanceLine2Box2Window2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual void DrawScreenOverlay() override;
    bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
#if defined(USE_QUERY_LINE)
    using LinearType = Line2<double>;
#endif
#if defined(USE_QUERY_RAY)
    using LinearType = Ray2<double>;
#endif
#if defined(USE_QUERY_SEGMENT)
    using LinearType = Segment2<double>;
#endif

#if defined(USE_QUERY_ALIGNED_BOX)
    using BoxType = AlignedBox2<double>;
#else
    using BoxType = OrientedBox2<double>;
#endif

    using Query = DCPQuery<double, LinearType, BoxType>;

    LinearType mLinear;
    BoxType mBox;
    Query mQuery;
    Query::Result mResult;
    double mAngle;
#if defined(USE_QUERY_SEGMENT)
    double mSegmentLength;
#endif
};
