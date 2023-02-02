// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

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
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void ComputeTruePath();
    void ComputeApprPath(bool subdivide);
    void ComputeApprLength();
    void ParamToXY(GVector<float> const& param, int32_t& x, int32_t& y);
    void XYToParam(int32_t x, int32_t y, GVector<float>& param);

    int32_t mSize;
    EllipsoidGeodesic<float> mGeodesic;
    GVector<float> mParam0, mParam1;
    float mXMin, mXMax, mXDelta;
    float mYMin, mYMax, mYDelta;

    int32_t mNumTruePoints;
    std::vector<GVector<float>> mTruePoints;
    int32_t mNumApprPoints;
    std::vector<GVector<float>> mApprPoints;

    int32_t mCurrNumApprPoints;
    float mTrueDistance;
    float mApprDistance;
    float mApprCurvature;

    std::array<float, 4> mTextColor;
};
