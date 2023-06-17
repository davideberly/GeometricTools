// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.06.16

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/IntrEllipse2Ellipse2.h>
using namespace gte;

class IntersectEllipsesWindow2 : public Window2
{
public:
    IntersectEllipsesWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual void DrawScreenOverlay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    inline void Get(Ellipse2<double> const& ellipse,
        size_t i, int32_t& x, int32_t& y)
    {
        Vector2<double> P = ellipse.center
            + (ellipse.extent[0] * mCosAngle[i]) * ellipse.axis[0]
            + (ellipse.extent[1] * mSinAngle[i]) * ellipse.axis[1];

        P += mOrigin;

        x = static_cast<int32_t>(P[0]);
        y = static_cast<int32_t>(P[1]);
    }

    void Translate(int32_t i, double trnDelta);
    void Rotate(double rotDelta);
    void DoQuery();

    std::array<Ellipse2<double>, 2> mEllipse;
    std::array<Vector2<double>, 2> mCenter;
    std::array<Matrix2x2<double>, 2> mMatrix;
    FIQuery<double, Ellipse2<double>, Ellipse2<double>> mQuery;
    FIQuery<double, Ellipse2<double>, Ellipse2<double>>::Result mResult;

    static size_t constexpr numAngles = 2048;
    std::vector<double> mCosAngle, mSinAngle;
    Vector2<double> mOrigin;

    std::array<double, 3> mTrnDelta;
    std::array<double, 3> mRotDelta;
    int32_t mSpeed;
    size_t mActive;
};
