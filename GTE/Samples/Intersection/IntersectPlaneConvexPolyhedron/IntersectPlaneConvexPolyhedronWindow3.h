// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/IntrConvexMesh3Plane3.h>
using namespace gte;

class IntersectPlaneConvexPolyhedronWindow3: public Window3
{
public:
    IntersectPlaneConvexPolyhedronWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    using Rational = BSRational<UIntegerAP32>;
    using CM = ConvexMesh3<Rational>;
    using Query = FIQuery<Rational, CM, Plane3<Rational>>;

    void CreateQueryObjects();
    void CreateScene();
    void DoQuery();
    void UpdatePlane();

    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<RasterizerState> mNoCullSolidState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<DepthStencilState> mDepthReadNoWriteState;
    std::shared_ptr<Visual> mPosPolyMesh;
    std::shared_ptr<Visual> mNegPolyMesh;
    std::shared_ptr<Visual> mPolygonCurve;
    std::shared_ptr<Visual> mPolygonMesh;
    std::shared_ptr<Visual> mPlaneMesh;
    float mAlpha;
    float mDeltaDistance, mDeltaTheta, mDeltaPhi, mDistance, mTheta, mPhi;
    bool mValidPosPolyMesh, mValidNegPolyMesh, mValidPolygonCurve, mValidPolygonMesh;
    bool mDrawPosPolyMesh, mDrawNegPolyMesh, mDrawPolygonCurve, mDrawPolygonMesh;

    CM mPolyhedron;
    Plane3<Rational> mPlane;
    Query mQuery;
    Query::Result mResult;
};
