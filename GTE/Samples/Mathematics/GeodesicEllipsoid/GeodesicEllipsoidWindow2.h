// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/EllipsoidGeodesic.h>
using namespace gte;

class GeodesicEllipsoidWindow2 : public Window2
{
public:
    GeodesicEllipsoidWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual void DrawScreenOverlay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void ComputeTruePath();
    void ComputeApprPath(bool subdivide);
    void ComputeApprLength();
    void ParamToXY(GVector<float> const& param, int& x, int& y);
    void XYToParam(int x, int y, GVector<float>& param);

    int mSize;
    EllipsoidGeodesic<float> mGeodesic;
    GVector<float> mParam0, mParam1;
    float mXMin, mXMax, mXDelta;
    float mYMin, mYMax, mYDelta;

    int mNumTruePoints;
    std::vector<GVector<float>> mTruePoints;
    int mNumApprPoints;
    std::vector<GVector<float>> mApprPoints;

    int mCurrNumApprPoints;
    float mTrueDistance;
    float mApprDistance;
    float mApprCurvature;

    std::array<float, 4> mTextColor;
};
