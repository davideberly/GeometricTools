// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.08.25

#pragma once

// The 3D line-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document. The circle has
// center C and the plane of the circle has unit-length normal N. The line has
// origin B and non-zero direction M. The parameterization is P(t) = t*M+B. It
// is not necessary that M be a unit-length vector. This allows for the
// line-circle query to be used in the segment-circle query for the two-point
// form of a segment where M is the difference of the endpoints, which avoids
// a normalization of M that has numerical rounding errors. The type T can be
// a floating-point type or a rational type.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Circle3.h>
#include <Mathematics/Line.h>
#include <Mathematics/Ray.h>
#include <Mathematics/Segment.h>
#include <Mathematics/RootsBisection1.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <functional>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Line3<T>, Circle3<T>>
    {
    public:
        // The possible number of closest line-circle pairs is 1, 2 or all
        // circle points. If 1 or 2, numClosestPairs is set to this number
        // and 'equidistant' is false; the number of valid elements in
        // linearClosest[] and circularClosest[] is numClosestPairs. If all
        // circle points are closest, the line must be C+s*N where C is the
        // circle center, N is the normal to the plane of the circle, and
        // linearClosest[0] is set to C. In this case, 'equidistant' is true
        // and circularClosest[0] is set to C+r*U, where r is the circle
        // radius and U is a vector perpendicular to N.
        //
        // This structure is also used by ray-circle and segment-circle
        // distance queries. For line-circle, linearClosest[] refers to the
        // closest line points. For ray-circle, linearClosest[] refers to the
        // closest ray points. For segment-circle, linearClosest[] refers to
        // the closest segment points. Sharing of Result avoids copying
        // between two structures that are nearly identical.
        struct Result
        {
            Result()
                :
                numClosestPairs(0),
                linearClosest{},
                circularClosest{},
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                equidistant(false)
            {
            }

            std::size_t numClosestPairs;
            std::array<Vector3<T>, 2> linearClosest, circularClosest;
            T distance, sqrDistance;
            bool equidistant;
        };

        Result operator()(Line3<T> const& line, Circle3<T> const& circle)
        {
            Result output{};
            Critical critical{};
            Execute(line, circle, output, critical);
            return output;
        }

    private:
        friend class DCPQuery<T, Ray3<T>, Circle3<T>>;
        friend class DCPQuery<T, Segment3<T>, Circle3<T>>;

        struct Critical
        {
            Critical()
                :
                numPoints(0),
                linearPoint{},
                circularPoint{},
                parameter{ static_cast<T>(0), static_cast<T>(0) },
                distance{ static_cast<T>(0), static_cast<T>(0) }
            {
            }

            std::size_t numPoints;
            std::array<Vector3<T>, 2> linearPoint, circularPoint;
            std::array<T, 2> parameter;
            std::array<T, 2> distance;
        };

        // The last 4 parameters are in-out variables. On input they should be
        // zero-valued.
        void Execute(Line3<T> const& line, Circle3<T> const& circle,
            Result& result, Critical& critical)
        {
            result = Result{};
            critical = Critical{};

            // Translate the line and circle so that the circle center is the
            // origin (0,0,0). D is the translated line origin.
            Vector3<T> const& N = circle.normal;
            Vector3<T> const& M = line.direction;
            Vector3<T> D = line.origin - circle.center;
            Vector3<T> NxM = Cross(N, M);
            Vector3<T> NxD = Cross(N, D);

            // The constructor sets vzero to (0,0,0).
            Vector3<T> const vzero{};
            if (NxM != vzero)
            {
                // The line is not perpendicular to the plane of the circle.
                if (NxD != vzero)
                {
                    // The line origin is not on the normal line through the
                    // circle center.
                    PDFSection422(line, circle, D, NxM, NxD, result, critical);
                }
                else
                {
                    // The line origin is on the normal line through the circle
                    // center.
                    PDFSection421(line, circle, D, NxM, result, critical);
                }
            }
            else
            {
                // The line is perpendicular to the plane of the circle.
                if (NxD != vzero)
                {
                    // The line does not contain the circle center.
                    PDFSection412(line, circle, D, result, critical);
                }
                else
                {
                    // The line contains the circle center.
                    PDFSection411(line, circle, D, result, critical);
                }
            }
        }

        // The line is perpendicular to the plane of the circle and contains
        // the circle center.
        void PDFSection411(Line3<T> const& line, Circle3<T> const& circle, Vector3<T> const& D,
            Result& result, Critical& critical)
        {
            // Convenient aliases for the line and circle quantities.
            Vector3<T> const& M = line.direction;
            Vector3<T> const& C = circle.center;
            Vector3<T> const& N = circle.normal;
            T const& r = circle.radius;

            result.numClosestPairs = 1;
            result.linearClosest[0] = C;
            Vector3<T> U = GetOrthogonal(N, true);
            result.circularClosest[0] = C + r * U;
            Vector3<T> diff = result.linearClosest[0] - result.circularClosest[0];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            result.equidistant = true;
            critical.numPoints = 1;
            critical.linearPoint[0] = result.linearClosest[0];
            critical.circularPoint[0] = result.circularClosest[0];
            critical.parameter[0] = -Dot(M, D) / Dot(M, M);
            critical.distance[0] = result.distance;
        }

        // The line is perpendicular to the plane of the circle and does not
        // contain the circle center.
        void PDFSection412(Line3<T> const& line, Circle3<T> const& circle, Vector3<T> const& D,
            Result& result, Critical& critical)
        {
            // Convenient alias for the line direction.
            Vector3<T> const& M = line.direction;

            critical.numPoints = 1;
            critical.parameter[0] = -Dot(M, D) / Dot(M, M);
            Finalize(line, circle, D, result, critical);
        }

        // The line is not perpendicular to the plane of the circle but the
        // line origin is on the normal line through the circle center.
        void PDFSection421(Line3<T> const& line, Circle3<T> const& circle,
            Vector3<T> const& D, Vector3<T> const& NxM,
            Result& result, Critical& critical)
        {
            // Convenient aliases for the line and circle quantities.
            Vector3<T> const& M = line.direction;
            T const& r = circle.radius;

            T MdD = Dot(M, D);
            T MdM = Dot(M, M);
            T rLenCrossMN = r * Length(NxM);
            critical.numPoints = 2;
            critical.parameter[0] = (-MdD - rLenCrossMN) / MdM;
            critical.parameter[1] = (-MdD + rLenCrossMN) / MdM;
            Finalize(line, circle, D, result, critical);
        }

        // The line is not perpendicular to the plane of the circle and the
        // line origin is not on the normal line through the circle center.
        void PDFSection422(Line3<T> const& line, Circle3<T> const& circle,
            Vector3<T> const& D, Vector3<T> const& NxM, Vector3<T> const& NxD,
            Result& result, Critical& critical)
        {
            // Convenient aliases for the line and circle quantities.
            Vector3<T> const& M = line.direction;
            Vector3<T> const& N = circle.normal;
            T const& r = circle.radius;

            // Choose a new line origin E so that P(t) = E + t * M and
            // Dot(NxM, NxE) = 0.
            T NxMdNxM = Dot(NxM, NxM);
            T s = -Dot(NxM, NxD) / NxMdNxM;
            Vector3<T> E = s * M + D;

            // Phi(t) = (t + a0) - a1 * t / (a2 * t^2 + a3)^{1/2}
            // G(t) = a1 * t / (a2 * t^2 + a3)^{1/2}
            // G'(t) = a1 * a2 / (a2 * t^2 + a3)^{3/2}
            // G"(t) = -3 * a1 * a2^2 * t / (a2 * t^2 + a3)^{5/2}
            T const zero = static_cast<T>(0);
            T MdM = Dot(M, M);
            Vector3<T> NxE = Cross(N, E);
            T a0 = Dot(M, E) / MdM;
            T a1 = r * NxMdNxM / MdM;   // a1 > 0
            T a2 = NxMdNxM;             // a2 > 0
            T a3 = Dot(NxE, NxE);       // a3 >= 0
            T tau{};

            if (a1 > std::sqrt(a3))
            {
                // G'(0) > 1, std::fabs guards against numerical rounding
                // errors causing the argument of std::sqrt to be negative.
                T const twoThirds = static_cast<T>(2) / static_cast<T>(3);
                T tauHat = std::sqrt(std::fabs(std::pow(a1 * a3, twoThirds) - a3));
                T gTauHat = a1 * tauHat / std::sqrt(a2 * tauHat * tauHat + a3);
                T intercept = gTauHat - tauHat;  // Theoretically positive.
                if (a0 <= -intercept)
                {
                    tau = Bisect(a0, a1, a2, a3, -a0, -a0 + a1 / std::sqrt(a2));
                    if (a0 < -intercept)
                    {
                        critical.numPoints = 1;
                        critical.parameter[0] = tau + s;
                    }
                    else
                    {
                        critical.numPoints = 2;
                        critical.parameter[0] = tau + s;
                        critical.parameter[1] = -tauHat + s;
                    }
                }
                else if (a0 >= intercept)
                {
                    tau = Bisect(a0, a1, a2, a3, -a0 - a1 / std::sqrt(a2), -a0);
                    if (a0 > intercept)
                    {
                        critical.numPoints = 1;
                        critical.parameter[0] = tau + s;
                    }
                    else
                    {
                        critical.numPoints = 2;
                        critical.parameter[0] = tauHat + s;
                        critical.parameter[1] = tau + s;
                    }
                }
                else
                {
                    critical.numPoints = 2;
                    if (a0 > zero)
                    {
                        tau = Bisect(a0, a1, a2, a3, -a0 - a1 / std::sqrt(a2), -a0);
                        critical.parameter[0] = tau + s;
                        tau = Bisect(a0, a1, a2, a3, tauHat, -a0 + a1 / std::sqrt(a2));
                        critical.parameter[1] = tau + s;
                    }
                    else if (a0 < zero)
                    {
                        tau = Bisect(a0, a1, a2, a3, -a0 - a1 / std::sqrt(a2), -tauHat);
                        critical.parameter[0] = tau + s;
                        tau = Bisect(a0, a1, a2, a3, -a0, -a0 + a1 / std::sqrt(a2));
                        critical.parameter[1] = tau + s;
                    }
                    else // a0 = 0
                    {
                        tau = std::sqrt((a1 * a1 - a3) / a2);
                        critical.parameter[0] = s - tau;
                        critical.parameter[1] = s + tau;
                    }
                }
            }
            else
            {
                // G'(0) <= 1
                if (a0 < zero)
                {
                    tau = Bisect(a0, a1, a2, a3, -a0, -a0 + a1 / std::sqrt(a2));
                }
                else if (a0 > zero)
                {
                    tau = Bisect(a0, a1, a2, a3, -a0 - a1 / std::sqrt(a2), -a0);
                }
                else
                {
                    tau = zero;
                }
                critical.numPoints = 1;
                critical.parameter[0] = tau + s;
            }

            Finalize(line, circle, D, result, critical);
        }

        void Finalize(Line3<T> const& line, Circle3<T> const& circle, Vector3<T> const& D,
            Result& result, Critical& critical)
        {
            for (std::size_t i = 0; i < critical.numPoints; ++i)
            {
                // Get the closest pair of line and circle points.
                critical.linearPoint[i] = critical.parameter[i] * line.direction + D;
                T dot = Dot(circle.normal, critical.linearPoint[i]);
                Vector3<T> project = critical.linearPoint[i] - dot * circle.normal;
                Normalize(project);
                critical.linearPoint[i] += circle.center;
                critical.circularPoint[i] = circle.center + circle.radius * project;
                Vector3<T> diff = critical.linearPoint[i] - critical.circularPoint[i];
                critical.distance[i] = Length(diff);
            }

            if (critical.numPoints == 1)
            {
                result.numClosestPairs = 1;
                result.distance = critical.distance[0];
                result.linearClosest[0] = critical.linearPoint[0];
                result.circularClosest[0] = critical.circularPoint[0];
            }
            else // critical.numPoints = 2
            {
                if (critical.distance[0] < critical.distance[1])
                {
                    result.numClosestPairs = 1;
                    result.distance = critical.distance[0];
                    result.linearClosest[0] = critical.linearPoint[0];
                    result.circularClosest[0] = critical.circularPoint[0];
                }
                else if (critical.distance[0] > critical.distance[1])
                {
                    result.numClosestPairs = 1;
                    result.distance = critical.distance[1];
                    result.linearClosest[0] = critical.linearPoint[1];
                    result.circularClosest[0] = critical.circularPoint[1];
                }
                else // critical.distance[0] = critical.distance[1]
                {
                    result.numClosestPairs = 2;
                    result.distance = critical.distance[0];
                    result.linearClosest = critical.linearPoint;
                    result.circularClosest = critical.circularPoint;
                }
            }

            result.sqrDistance = result.distance * result.distance;
        }

        // Bisect the function Phi(t) = t + a0 - a1 * t / sqrt(a2 * t^2 + a3)
        // on the specified interval [tmin,tmax].
        T Bisect(T const& a0, T const& a1, T const& a2, T const& a3, T const& tauMin, T const& tauMax)
        {
            // Bisection using double precision is much faster than using
            // exact rational numbers.
            std::size_t constexpr maxIterations = 4096;
            double dA0 = static_cast<double>(a0);
            double dA1 = static_cast<double>(a1);
            double dA2 = static_cast<double>(a2);
            double dA3 = static_cast<double>(a3);
            double dTauMin = static_cast<double>(tauMin);
            double dTauMax = static_cast<double>(tauMax);

            std::function<double(double const&)> Phi = [&, dA0, dA1, dA2, dA3](double const& tau)
                {
                    return tau + dA0 - dA1 * tau / std::sqrt(dA2 * tau * tau + dA3);
                };

            // The function is known to be increasing, so we can specify -1 and +1
            // as the function values at the bounding interval endpoints.
            double dRoot = 0.0, dFRoot = 0.0;
            RootsBisection1<double> bisector(maxIterations);
            bisector(Phi, dTauMin, dTauMax, -1.0, 1.0, dRoot, dFRoot);
            (void)dFRoot;
            return static_cast<T>(dRoot);
        }
    };
}
