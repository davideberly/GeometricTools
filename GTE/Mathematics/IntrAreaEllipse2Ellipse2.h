// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

// Compute the area of intersection for two ellipses in 2D. The algorithm is
// discussed in the document
//   https://www.geometrictools.com/Documentation/AreaIntersectingEllipses.pdf

#include <Mathematics/IntrEllipse2Ellipse2.h>
#include <array>
#include <cmath>
#include <cstdint>
#include <map>
#include <utility>

namespace gte
{
    template <typename T>
    class AreaEllipse2Ellipse2
    {
    public:
        using EIQuery = FIQuery<T, Ellipse2<T>, Ellipse2<T>>;

        // Compute the area of intersection of ellipses.
        struct Result
        {
            // The configuration of the two ellipses.
            enum class Configuration
            {
                ELLIPSES_ARE_EQUAL,
                ELLIPSES_ARE_SEPARATED,
                E0_CONTAINS_E1,
                E1_CONTAINS_E0,
                ONE_CHORD_REGION,
                FOUR_CHORD_REGION,
                INVALID
            };

            Result()
                :
                configuration(Configuration::INVALID),
                findResult{},
                area((T)0)
            {
            }

            // One of the enumerates, determined in the call to AreaDispatch.
            Configuration configuration;

            // Information about the ellipse-ellipse intersection points.
            typename EIQuery::Result findResult;

            // The area of intersection of the ellipses.
            T area;
        };

        // The ellipse axes are not required to be normalized. The ellipse
        // has a rational representation.
        Result operator()(Ellipse2<T> const& ellipse0, Ellipse2<T> const& ellipse1)
        {
            EllipseInfo E0{};
            E0.center = ellipse0.center;
            E0.axis = ellipse0.axis;
            E0.extent = ellipse0.extent;
            FinishEllipseInfo(E0);

            EllipseInfo E1{};
            E1.center = ellipse1.center;
            E1.axis = ellipse1.axis;
            E1.extent = ellipse1.extent;
            FinishEllipseInfo(E1);

            Result ar{};
            ar.configuration = Result::Configuration::INVALID;
            ar.findResult = EIQuery{}(ellipse0, ellipse1);
            ar.area = mZero;
            AreaDispatch(E0, E1, ar);
            return ar;
        }

    private:
        struct EllipseInfo
        {
            EllipseInfo()
                :
                center(Vector2<T>::Zero()),
                axis{ Vector2<T>::Zero() , Vector2<T>::Zero() },
                extent(Vector2<T>::Zero()),
                M{},
                AB((T)0),
                halfAB((T)0),
                BpA((T)0),
                BmA((T)0)
            {
            }

            Vector2<T> center;
            std::array<Vector2<T>, 2> axis;
            Vector2<T> extent;
            Matrix2x2<T> M;
            T AB;        // extent[0] * extent[1]
            T halfAB;    // extent[0] * extent[1] / 2
            T BpA;       // extent[1] + extent[0]
            T BmA;       // extent[1] - extent[0]
        };

        void FinishEllipseInfo(EllipseInfo& E)
        {
            E.M = OuterProduct(E.axis[0], E.axis[0]) /
                (E.extent[0] * E.extent[0] * Dot(E.axis[0], E.axis[0]));
            E.M += OuterProduct(E.axis[1], E.axis[1]) /
                (E.extent[1] * E.extent[1] * Dot(E.axis[1], E.axis[1]));
            E.AB = E.extent[0] * E.extent[1];
            E.halfAB = E.AB / mTwo;
            E.BpA = E.extent[1] + E.extent[0];
            E.BmA = E.extent[1] - E.extent[0];
        }

        void AreaDispatch(EllipseInfo const& E0, EllipseInfo const& E1, Result& ar)
        {
            if (ar.findResult.intersect)
            {
                if (ar.findResult.numPoints == 1)
                {
                    // Containment or separation.
                    AreaCS(E0, E1, ar);
                }
                else if (ar.findResult.numPoints == 2)
                {
                    if (ar.findResult.isTransverse[0])
                    {
                        // Both intersection points are transverse.
                        Area2(E0, E1, 0, 1, ar);
                    }
                    else
                    {
                        // Both intersection points are tangential, so one
                        // ellipse is contained in the other.
                        AreaCS(E0, E1, ar);
                    }
                }
                else if (ar.findResult.numPoints == 3)
                {
                    // The tangential intersection is irrelevant in the area
                    // computation.
                    if (!ar.findResult.isTransverse[0])
                    {
                        Area2(E0, E1, 1, 2, ar);
                    }
                    else if (!ar.findResult.isTransverse[1])
                    {
                        Area2(E0, E1, 2, 0, ar);
                    }
                    else  // ar.findResult.isTransverse[2] == false
                    {
                        Area2(E0, E1, 0, 1, ar);
                    }
                }
                else  // ar.findResult.numPoints == 4
                {
                    Area4(E0, E1, ar);
                }
            }
            else
            {
                // Containment, separation, or same ellipse.
                AreaCS(E0, E1, ar);
            }
        }

        void AreaCS(EllipseInfo const& E0, EllipseInfo const& E1, Result& ar)
        {
            if (ar.findResult.numPoints <= 1)
            {
                Vector2<T> diff = E0.center - E1.center;
                T qform0 = Dot(diff, E0.M * diff);
                T qform1 = Dot(diff, E1.M * diff);
                if (qform0 > mOne && qform1 > mOne)
                {
                    // Each ellipse center is outside the other ellipse, so
                    // the ellipses are separated (numPoints == 0) or outside
                    // each other and just touching (numPoints == 1).
                    ar.configuration = Result::Configuration::ELLIPSES_ARE_SEPARATED;
                    ar.area = mZero;
                }
                else
                {
                    // One ellipse is inside the other.  Determine this
                    // indirectly by comparing areas.
                    if (E0.AB < E1.AB)
                    {
                        ar.configuration = Result::Configuration::E1_CONTAINS_E0;
                        ar.area = mPi * E0.AB;
                    }
                    else
                    {
                        ar.configuration = Result::Configuration::E0_CONTAINS_E1;
                        ar.area = mPi * E1.AB;
                    }
                }
            }
            else
            {
                ar.configuration = Result::Configuration::ELLIPSES_ARE_EQUAL;
                ar.area = mPi * E0.AB;
            }
        }

        void Area2(EllipseInfo const& E0, EllipseInfo const& E1, int32_t i0, int32_t i1, Result& ar)
        {
            ar.configuration = Result::Configuration::ONE_CHORD_REGION;

            // The endpoints of the chord.
            Vector2<T> const& P0 = ar.findResult.points[i0];
            Vector2<T> const& P1 = ar.findResult.points[i1];

            // Compute locations relative to the ellipses.
            Vector2<T> P0mC0 = P0 - E0.center, P0mC1 = P0 - E1.center;
            Vector2<T> P1mC0 = P1 - E0.center, P1mC1 = P1 - E1.center;

            // Compute the ellipse normal vectors at endpoint P0.  This is
            // sufficient information to determine chord endpoint order.
            Vector2<T> N0 = E0.M * P0mC0, N1 = E1.M * P0mC1;
            T dotperp = DotPerp(N1, N0);

            // Choose the endpoint order for the chord region associated
            // with E0.
            if (dotperp > mZero)
            {
                // The chord order for E0 is <P0,P1> and for E1 is <P1,P0>.
                ar.area =
                    ComputeAreaChordRegion(E0, P0mC0, P1mC0) +
                    ComputeAreaChordRegion(E1, P1mC1, P0mC1);
            }
            else
            {
                // The chord order for E0 is <P1,P0> and for E1 is <P0,P1>.
                ar.area =
                    ComputeAreaChordRegion(E0, P1mC0, P0mC0) +
                    ComputeAreaChordRegion(E1, P0mC1, P1mC1);
            }
        }

        void Area4(EllipseInfo const& E0, EllipseInfo const& E1, Result& ar)
        {
            ar.configuration = Result::Configuration::FOUR_CHORD_REGION;

            // Select a counterclockwise ordering of the points of
            // intersection.  Use the polar coordinates for E0 to do this.
            // The multimap is used in the event that computing the
            // intersections involved numerical rounding errors that lead to
            // a duplicate intersection, even though the intersections are all
            // labeled as transverse.  [See the comment in the function
            // SpecialIntersection.]
            std::multimap<T, int32_t> ordering;
            int32_t i;
            for (i = 0; i < 4; ++i)
            {
                Vector2<T> PmC = ar.findResult.points[i] - E0.center;
                T x = Dot(E0.axis[0], PmC);
                T y = Dot(E0.axis[1], PmC);
                T theta = std::atan2(y, x);
                ordering.insert(std::make_pair(theta, i));
            }

            std::array<int32_t, 4> permute{ 0, 0, 0, 0 };
            i = 0;
            for (auto const& element : ordering)
            {
                permute[i++] = element.second;
            }

            // Start with the area of the convex quadrilateral.
            Vector2<T> diag20 =
                ar.findResult.points[permute[2]] - ar.findResult.points[permute[0]];
            Vector2<T> diag31 =
                ar.findResult.points[permute[3]] - ar.findResult.points[permute[1]];
            ar.area = std::fabs(DotPerp(diag20, diag31)) / mTwo;

            // Visit each pair of consecutive points.  The selection of
            // ellipse for the chord-region area calculation uses the "most
            // counterclockwise" tangent vector.
            for (int32_t i0 = 3, i1 = 0; i1 < 4; i0 = i1++)
            {
                // Get a pair of consecutive points.
                Vector2<T> const& P0 = ar.findResult.points[permute[i0]];
                Vector2<T> const& P1 = ar.findResult.points[permute[i1]];

                // Compute locations relative to the ellipses.
                Vector2<T> P0mC0 = P0 - E0.center, P0mC1 = P0 - E1.center;
                Vector2<T> P1mC0 = P1 - E0.center, P1mC1 = P1 - E1.center;

                // Compute the ellipse normal vectors at endpoint P0.
                Vector2<T> N0 = E0.M * P0mC0, N1 = E1.M * P0mC1;
                T dotperp = DotPerp(N1, N0);
                if (dotperp > mZero)
                {
                    // The chord goes with ellipse E0.
                    ar.area += ComputeAreaChordRegion(E0, P0mC0, P1mC0);
                }
                else
                {
                    // The chord goes with ellipse E1.
                    ar.area += ComputeAreaChordRegion(E1, P0mC1, P1mC1);
                }
            }
        }

        T ComputeAreaChordRegion(EllipseInfo const& E, Vector2<T> const& P0mC, Vector2<T> const& P1mC)
        {
            // Compute polar coordinates for P0 and P1 on the ellipse.
            T x0 = Dot(E.axis[0], P0mC);
            T y0 = Dot(E.axis[1], P0mC);
            T theta0 = std::atan2(y0, x0);
            T x1 = Dot(E.axis[0], P1mC);
            T y1 = Dot(E.axis[1], P1mC);
            T theta1 = std::atan2(y1, x1);

            // The arc straddles the atan2 discontinuity on the negative
            // x-axis.  Wrap the second angle to be larger than the first
            // angle.
            if (theta1 < theta0)
            {
                theta1 += mTwoPi;
            }

            // Compute the area portion of the sector due to the triangle.
            T triArea = std::fabs(DotPerp(P0mC, P1mC)) / mTwo;

            // Compute the chord region area.
            T dtheta = theta1 - theta0;
            T F0, F1, sectorArea;
            if (dtheta <= mPi)
            {
                // Use the area formula directly.
                // area(theta0,theta1) = F(theta1)-F(theta0)-area(triangle)
                F0 = ComputeIntegral(E, theta0);
                F1 = ComputeIntegral(E, theta1);
                sectorArea = F1 - F0;
                return sectorArea - triArea;
            }
            else
            {
                // The angle of the elliptical sector is larger than pi
                // radians.  Use the area formula
                //   area(theta0,theta1) = pi*a*b - area(theta1,theta0)
                theta0 += mTwoPi;  // ensure theta0 > theta1
                F0 = ComputeIntegral(E, theta0);
                F1 = ComputeIntegral(E, theta1);
                sectorArea = F0 - F1;
                return mPi * E.AB - (sectorArea - triArea);
            }
        }

        T ComputeIntegral(EllipseInfo const& E, T const& theta)
        {
            T twoTheta = mTwo * theta;
            T sn = std::sin(twoTheta);
            T cs = std::cos(twoTheta);
            T arg = E.BmA * sn / (E.BpA + E.BmA * cs);
            return E.halfAB * (theta - std::atan(arg));
        }

        // Constants that are set up once (optimization for rational
        // arithmetic).
        T mZero, mOne, mTwo, mPi, mTwoPi;
    };
}
