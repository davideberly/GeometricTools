// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "DistancePointHyperellipsoidConsole.h"
#include <Mathematics/DistPointHyperellipsoid.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/Vector3.h>
#include <fstream>
using namespace gte;

DistancePointHyperellipsoidConsole::DistancePointHyperellipsoidConsole(Parameters& parameters)
    :
    Console(parameters)
{
}

void DistancePointHyperellipsoidConsole::Execute()
{
    TestDistancePointEllipse();
    TestDistancePointEllipsoid();
}

void DistancePointHyperellipsoidConsole::TestDistancePointEllipse()
{
    std::ofstream output("TestEllipse.txt");

    Ellipse2<double> ellipse;
    ellipse.center = { 1.0, 2.0 };
    ellipse.axis[0] = { 2.0, 1.0 };
    ComputeOrthogonalComplement(1, ellipse.axis.data());
    ellipse.extent[0] = 0.6f;
    ellipse.extent[1] = 1.2f;

    // The ellipse is defined implicitly by
    //   Q(x,y) = c[0] + c[1]*x + c[2]*y + c[3]*x^2 + c[4]*x*y + c[5]*y^2 = 0
    // A normal vector at (x,y) is
    //   grad[Q](x,y) = (c[1] + 2*c[3]*x + c[4]*y, c[2] + c[3]*x + 2*c[5]*y)
    std::array<double, 6> c;
    ellipse.ToCoefficients(c);

    double xExtreme = 2.0f;
    double yExtreme = 1.0f;
    int32_t const numXSamples = 32;
    int32_t const numYSamples = 16;
    Vector2<double> point, K, grad, diff;
    double dot, angle;
    DCPQuery<double, Vector2<double>, Ellipse2<double>> query;
    for (int32_t y = 0; y < numYSamples; ++y)
    {
        point[1] = -yExtreme + 2.0 * yExtreme * y / (numYSamples - 1.0);
        for (int32_t x = 0; x < numXSamples; ++x)
        {
            point[0] = -xExtreme + 2.0 * xExtreme * x / (numXSamples - 1.0);

            auto result = query(point, ellipse);
            K = result.closest[1];

            // Compute the angle between grad[Q](x,y) and (x,y)-closest(x,y).
            grad[0] = c[1] + 2.0 * c[3] * K[0] + c[4] * K[1];
            grad[1] = c[2] + c[4] * K[0] + 2.0 * c[5] * K[1];
            Normalize(grad);
            diff = point - K;
            Normalize(diff);
            dot = Dot(grad, diff);
            if (dot < 0.0)
            {
                grad = -grad;
                dot = -dot;
            }

            angle = std::acos(std::min(std::max(dot, -1.0), 1.0));

            output << "(x,y) = (" << point[0] << ", " << point[1] << "); ";
            output << "(kx,ky) = (" << K[0] << ", " << K[1] << "); ";
            output << "distance = " << result.distance << "; ";
            output << "diff = (" << diff[0] << ", " << diff[1] << "); ";
            output << "grad = (" << grad[0] << ", " << grad[1] << "); ";
            output << "angle = " << angle << std::endl;
        }
    }

    output.close();
}

void DistancePointHyperellipsoidConsole::TestDistancePointEllipsoid()
{
    std::ofstream output("TestEllipsoid.txt");

    Ellipsoid3<double> ellipsoid;
    ellipsoid.center = { 1.0, 2.0, 3.0 };
    ellipsoid.axis[0] = { 3.0, 2.0, 1.0 };
    ComputeOrthogonalComplement(1, ellipsoid.axis.data());
    ellipsoid.extent[0] = 0.6f;
    ellipsoid.extent[1] = 1.2f;
    ellipsoid.extent[2] = 0.9f;

    // The ellipsoid is defined implicitly by
    //   Q(x,y,z) = c[0] + c[1]*x + c[2]*y + c[3]*z
    //     + c[4]*x^2 + c[5]*x*y + c[6]*x*z + c[7]*y^2
    //     + c[8]*y*z + c[9]*z^2 = 0
    // A normal vector at (x,y,z) is
    //   grad[Q](x,y,z) = (
    //     c[1] + 2*c[4]*x + c[5]*y + c[6]*z,
    //     c[2] + c[5]*x + 2*c[7]*y + c[8]*z,
    //     c[3] + c[6]*x + c[8]*y + 2*c[9]*z)
    std::array<double, 10> c;
    ellipsoid.ToCoefficients(c);

    double xExtreme = 2.0f;
    double yExtreme = 4.0f;
    double zExtreme = 3.0f;
    int32_t const numXSamples = 32;
    int32_t const numYSamples = 64;
    int32_t const numZSamples = 48;
    Vector3<double> point, K, grad, diff;
    double dot, angle;
    DCPQuery<double, Vector3<double>, Ellipsoid3<double>> query;
    for (int32_t z = 0; z < numZSamples; ++z)
    {
        point[2] = -zExtreme + 2.0 * zExtreme * z / (numZSamples - 1.0);
        for (int32_t y = 0; y < numYSamples; ++y)
        {
            point[1] = -yExtreme + 2.0 * yExtreme * y / (numYSamples - 1.0);
            for (int32_t x = 0; x < numXSamples; ++x)
            {
                point[0] = -xExtreme + 2.0 * xExtreme * x / (numXSamples - 1.0);

                auto result = query(point, ellipsoid);
                K = result.closest[1];

                // Compute the angle between grad[Q](x,y) and (x,y)-closest(x,y).
                grad[0] = c[1] + 2.0 * c[4] * K[0] + c[5] * K[1] + c[6] * K[2];
                grad[1] = c[2] + c[5] * K[0] + 2.0 * c[7] * K[1] + c[8] * K[2];
                grad[2] = c[3] + c[6] * K[0] + c[8] * K[1] + 2.0 * c[9] * K[2];
                Normalize(grad);
                diff = point - K;
                Normalize(diff);
                dot = Dot(grad, diff);
                if (dot < 0.0)
                {
                    grad = -grad;
                    dot = -dot;
                }

                angle = std::acos(std::min(std::max(dot, -1.0), 1.0));

                output << "(x,y,z) = (" << point[0] << ", " << point[1] << ", " << point[2] << "); ";
                output << "(kx,ky) = (" << K[0] << ", " << K[1] << ", " << K[2] << "); ";
                output << "distance = " << result.distance << "; ";
                output << "diff = (" << diff[0] << ", " << diff[1] << ", " << diff[2] << "); ";
                output << "grad = (" << grad[0] << ", " << grad[1] << ", " << grad[2] << "); ";
                output << "angle = " << angle << std::endl;
            }
        }
    }

    output.close();
}
