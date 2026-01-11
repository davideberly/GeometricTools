// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/ApprEllipse2.h>
using namespace gte;

class ApproximateEllipse2Window2 : public Window2
{
public:
    ApproximateEllipse2Window2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual void DrawScreenOverlay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void Get(Vector2<double> const& source, int32_t& x, int32_t& y) const
    {
        auto p = 64.0 * source + Vector2<double>{ 384.0, 384.0 };
        x = static_cast<int32_t>(p[0]);
        y = static_cast<int32_t>(p[1]);
    }

    Vector2<double> GetEllipsePoint(Ellipse2<double> const& ellipse, size_t imax, size_t i) const
    {
        double angle = GTE_C_TWO_PI * (double)i / (double)imax;
        double cs = std::cos(angle);
        double sn = std::sin(angle);
        Vector2<double> p = ellipse.center +
            ellipse.extent[0] * cs * ellipse.axis[0] +
            ellipse.extent[1] * sn * ellipse.axis[1];
        return p;
    }

    void DrawMyEllipse(Ellipse2<double> const& ellipse, size_t numSamples, uint32_t color)
    {
        int32_t x0{}, y0{}, x1{}, y1{};
        Get(GetEllipsePoint(ellipse, numSamples, 0), x0, y0);
        for (size_t i = 1; i < numSamples; ++i)
        {
            Get(GetEllipsePoint(ellipse, numSamples, i), x1, y1);
            DrawLine(x0, y0, x1, y1, color);
            x0 = x1;
            y0 = y1;
        }
        Get(GetEllipsePoint(ellipse, numSamples, 0), x1, y1);
        DrawLine(x0, y0, x1, y1, color);
    }

    ApprEllipse2<double> mFitter;
    std::vector<Vector2<double>> mPoints;
    Ellipse2<double> mTrueEllipse, mApprEllipse;
    size_t mIteration, mNumIterations;
    double mError;
};


