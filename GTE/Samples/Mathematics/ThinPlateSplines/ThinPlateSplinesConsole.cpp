// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "ThinPlateSplinesConsole.h"
#include <Mathematics/IntpThinPlateSpline2.h>
#include <Mathematics/IntpThinPlateSpline3.h>
#include <iomanip>
#include <random>

ThinPlateSplinesConsole::ThinPlateSplinesConsole(Parameters& parameters)
    :
    Console(parameters)
{
}

void ThinPlateSplinesConsole::Execute()
{
    TestThinPlateSplines2D();
    TestThinPlateSplines3D();
}

void ThinPlateSplinesConsole::TestThinPlateSplines2D()
{
    std::ofstream output("output2.txt");
    output << std::scientific << std::setprecision(8);

    // Tabulated data on a 3x3 regular grid, points of form (x,y,f(x,y)).
    int32_t const numPoints = 9;
    std::array<double, numPoints> x = { 0.0, 0.5, 1.0, 0.0, 0.5, 1.0, 0.0, 0.5, 1.0 };
    std::array<double, numPoints> y = { 0.0, 0.0, 0.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0 };
    std::array<double, numPoints> f = { 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 1.0, 2.0, 3.0 };

    // Resample on a 7x7 regular grid.
    int32_t const numResample = 6;
    double const invResample = 1.0 / static_cast<double>(numResample);
    double smooth, interp, functional;

    // No smoothing, exact interpolation at grid points.
    smooth = 0.0;
    IntpThinPlateSpline2<double> noSmooth(
        numPoints, x.data(), y.data(), f.data(), smooth, false);
    output << "no smoothing (smooth parameter is 0.0)" << std::endl;
    for (int32_t j = 0; j <= numResample; ++j)
    {
        for (int32_t i = 0; i <= numResample; ++i)
        {
            interp = noSmooth(invResample * i, invResample * j);
            output << interp << " ";
        }
        output << std::endl;
    }
    functional = noSmooth.ComputeFunctional();
    output << "functional = " << functional << std::endl << std::endl;

    // Increasing amounts of smoothing.
    smooth = 0.1;
    for (int32_t k = 1; k <= 6; ++k, smooth *= 10.0)
    {
        IntpThinPlateSpline2<double> spline(
            numPoints, x.data(), y.data(), f.data(), smooth, false);
        output << "smoothing (parameter is " << smooth << ")" << std::endl;
        for (int32_t j = 0; j <= numResample; ++j)
        {
            for (int32_t i = 0; i <= numResample; ++i)
            {
                interp = spline(invResample * i, invResample * j);
                interp = noSmooth(invResample * i, invResample * j);
                output << interp << " ";
            }
            output << std::endl;
        }
        functional = noSmooth.ComputeFunctional();
        output << "functional = " << functional << std::endl << std::endl;
    }

    output.close();
}

void ThinPlateSplinesConsole::TestThinPlateSplines3D()
{
    std::ofstream output("output3.txt");
    output << std::scientific << std::setprecision(8);

    // Tabulated data on a 3x3x3 regular grid, points (x,y,z,f(x,y,z)).
    std::default_random_engine dre;
    std::uniform_real_distribution<double> urd(0.0, 1.0);
    int32_t const numPoints = 27;
    std::array<double, numPoints> x, y, z, f;
    double xdomain, ydomain, zdomain;
    for (int32_t k = 0, index = 0; k < 3; ++k)
    {
        zdomain = 0.5 * k;
        for (int32_t j = 0; j < 3; ++j)
        {
            ydomain = 0.5 * j;
            for (int32_t i = 0; i < 3; ++i, ++index)
            {
                xdomain = 0.5 * i;
                x[index] = xdomain;
                y[index] = ydomain;
                z[index] = zdomain;
                f[index] = urd(dre);
            }
        }
    }

    // Resample on a 7x7x7 regular grid.
    int32_t const numResample = 6;
    double const invResample = 1.0 / static_cast<double>(numResample);
    double smooth, interp, functional;

    // No smoothing, exact interpolation at grid points.
    smooth = 0.0;
    IntpThinPlateSpline3<double> noSmooth(
        numPoints, x.data(), y.data(), z.data(), f.data(), smooth, false);
    output << "no smoothing (smooth parameter is 0.0)" << std::endl;
    for (int32_t k = 0; k <= numResample; ++k)
    {
        zdomain = invResample * k;
        for (int32_t j = 0; j <= numResample; ++j)
        {
            ydomain = invResample * j;
            for (int32_t i = 0; i <= numResample; ++i)
            {
                xdomain = invResample * i;
                interp = noSmooth(xdomain, ydomain, zdomain);
                output << interp << " ";
            }
            output << std::endl;
        }
        output << std::endl;
    }
    functional = noSmooth.ComputeFunctional();
    output << "functional = " << functional << std::endl << std::endl;

    // Increasing amounts of smoothing.
    smooth = 0.1;
    for (int32_t ell = 1; ell <= 6; ++ell, smooth *= 10.0)
    {
        IntpThinPlateSpline3<double> spline(
            numPoints, x.data(), y.data(), z.data(), f.data(), smooth, false);
        output << "smoothing (parameter is " << smooth << ")" << std::endl;
        for (int32_t k = 0; k <= numResample; ++k)
        {
            zdomain = invResample * k;
            for (int32_t j = 0; j <= numResample; ++j)
            {
                ydomain = invResample * j;
                for (int32_t i = 0; i <= numResample; ++i)
                {
                    xdomain = invResample * i;
                    interp = spline(xdomain, ydomain, zdomain);
                    output << interp << " ";
                }
                output << std::endl;
            }
            output << std::endl;
        }
        functional = noSmooth.ComputeFunctional();
        output << "functional = " << functional << std::endl << std::endl;
    }

    output.close();
}
