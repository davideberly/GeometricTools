// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a line and a solid aligned box in 2D.
//
// The line is P + t * D, where D is not required to be unit length.
// 
// The aligned box has minimum corner A and maximum corner B. A box point is X
// where A <= X <= B; the comparisons are componentwise.
// 
// The closest point on the line is stored in closest[0] with parameter t. The
// closest point on the box is stored in closest[1]. When there are infinitely
// many choices for the pair of closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/AlignedBox.h>
#include <GTL/Mathematics/Primitives/ND/Line.h>
#include <GTL/Mathematics/Primitives/ND/OrientedBox.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Line2<T>, AlignedBox2<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                parameter(C_<T>(0)),
                closest{}
            {
            }

            T distance, sqrDistance;
            T parameter;
            std::array<Vector2<T>, 2> closest;
        };

        Output operator()(Line2<T> const& line, AlignedBox2<T> const& box)
        {
            Output output{};

            // Translate the line and box so that the box has center at the
            // origin.
            Vector2<T> boxCenter{}, boxExtent{};
            box.GetCenteredForm(boxCenter, boxExtent);
            Vector2<T> origin = line.origin - boxCenter;
            Vector2<T> direction = line.direction;

            // The query computes 'output' relative to the box with center
            // at the origin.
            DoQuery(origin, direction, boxExtent, output);

            // Translate the closest points to the original coordinates.
            for (size_t i = 0; i < 2; ++i)
            {
                output.closest[i] += boxCenter;
            }

            // Compute the distance and squared distance.
            Vector2<T> diff = output.closest[0] - output.closest[1];
            output.sqrDistance = Dot(diff, diff);
            output.distance = std::sqrt(output.sqrDistance);
            return output;
        }

    protected:
        // Allow the line-oriented_box query to access DoQuery without having
        // to use class derivation.
        friend class DCPQuery<T, Line2<T>, OrientedBox2<T>>;

        // Compute the distance and closest point between a line and an
        // aligned box whose center is the origin. The origin and direction
        // are not const to allow for reflections that eliminate complicated
        // sign logic in the queries themselves.
        static void DoQuery(Vector2<T>& origin, Vector2<T>& direction,
            Vector2<T> const& extent, Output& output)
        {
            // Apply reflections so that the direction has nonnegative
            // components.
            std::array<bool, 2> reflect{ false, false };
            for (size_t i = 0; i < 2; ++i)
            {
                if (direction[i] < C_<T>(0))
                {
                    origin[i] = -origin[i];
                    direction[i] = -direction[i];
                    reflect[i] = true;
                }
            }

            // Compute the line-box distance and closest points. The DoQueryND
            // calls compute output.parameter and output.closest[1]. The
            // output.closest[0] can be computed after these calls.
            if (direction[0] > C_<T>(0))
            {
                if (direction[1] > C_<T>(0))
                {
                    // The direction signs are (+,+). If the line does not
                    // intersect the box, the only possible closest box points
                    // are K[0] = (-e0,e1) or K[1] = (e0,-e1). If the line
                    // intersects the box, the closest points are the same and
                    // chosen to be the intersection with box edge x0 = e0 or
                    // x1 = e1. For the remaining discussion, define K[2] =
                    // (e0,e1).
                    //
                    // Test where the candidate corners are relative to the
                    // line. If D = (d0,d1), then Perp(D) = (d1,-d0). The
                    // corner K[i] = P + t[i] * D + s[i] * Perp(D), where
                    // s[i] = Dot(K[i]-P,Perp(D))/|D|^2. K[0] is closest when
                    // s[0] >= 0 or K[1] is closest when s[1] <= 0. Otherwise,
                    // the line intersects the box. If s[2] >= 0, the common
                    // closest point is chosen to be (p0+(e1-p1)*d0/d1,e1). If
                    // s[2] < 0, the common closest point is chosen to be
                    // (e0,p1+(e0-p0)*d1/d0).
                    // 
                    // It is sufficient to test the signs of Dot(K[i],Perp(D))
                    // and defer the division by |D|^2 until needed for
                    // computing the closest point.
                    DoQuery2D(origin, direction, extent, output);
                }
                else
                {
                    // The direction signs are (+,0). The parameter is the
                    // value of t for which P + t * D = (e0, p1).
                    DoQuery1D(0, 1, origin, direction, extent, output);
                }
            }
            else
            {
                if (direction[1] > C_<T>(0))
                {
                    // The direction signs are (0,+). The parameter is the
                    // value of t for which P + t * D = (p0, e1).
                    DoQuery1D(1, 0, origin, direction, extent, output);
                }
                else
                {
                    // The direction signs are (0,0). The line is degenerate
                    // to a point (its origin). Clamp the origin to the box
                    // to obtain the closest point.
                    DoQuery0D(origin, extent, output);
                }
            }

            output.closest[0] = origin + output.parameter * direction;

            // Undo the reflections. The origin and direction are not consumed
            // by the caller, so these do not need to be reflected. However,
            // the closest points are consumed.
            for (size_t i = 0; i < 2; ++i)
            {
                if (reflect[i])
                {
                    for (size_t j = 0; j < 2; ++j)
                    {
                        output.closest[j][i] = -output.closest[j][i];
                    }
                }
            }
        }

    private:
        // Allow the line-oriented_box query to access DoQuery without having
        // to use class derivation.
        friend class DCPQuery<T, Line2<T>, OrientedBox2<T>>;

        static void DoQuery2D(Vector2<T> const& origin, Vector2<T> const& direction,
            Vector2<T> const& extent, Output& output)
        {
            Vector2<T> K0{ -extent[0], extent[1] };
            Vector2<T> delta = K0 - origin;
            T K0dotPerpD = DotPerp(delta, direction);
            if (K0dotPerpD >= C_<T>(0))
            {
                output.parameter = Dot(delta, direction) / Dot(direction, direction);
                output.closest[0] = origin + output.parameter * direction;
                output.closest[1] = K0;
            }
            else
            {
                Vector2<T> K1{ extent[0], -extent[1] };
                delta = K1 - origin;
                T K1dotPerpD = DotPerp(delta, direction);
                if (K1dotPerpD <= C_<T>(0))
                {
                    output.parameter = Dot(delta, direction) / Dot(direction, direction);
                    output.closest[0] = origin + output.parameter * direction;
                    output.closest[1] = K1;
                }
                else
                {
                    Vector2<T> K2{ extent[0], extent[1] };
                    delta = K2 - origin;
                    T K2dotPerpD = DotPerp(delta, direction);
                    if (K2dotPerpD >= C_<T>(0))
                    {
                        output.parameter = (extent[1] - origin[1]) / direction[1];
                        output.closest[0] = origin + output.parameter * direction;
                        output.closest[1][0] = origin[0] + output.parameter * direction[0];
                        output.closest[1][1] = extent[1];
                    }
                    else
                    {
                        output.parameter = (extent[0] - origin[0]) / direction[0];
                        output.closest[0] = origin + output.parameter * direction;
                        output.closest[1][0] = extent[0];
                        output.closest[1][1] = origin[1] + output.parameter * direction[1];
                    }
                }
            }
        }

        static void DoQuery1D(size_t i0, size_t i1, Vector2<T> const& origin,
            Vector2<T> const& direction, Vector2<T> const& extent, Output& output)
        {
            output.parameter = (extent[i0] - origin[i0]) / direction[i0];
            output.closest[0] = origin + output.parameter * direction;
            output.closest[1][i0] = extent[i0];
            output.closest[1][i1] = std::max(-extent[i1], std::min(origin[i1], extent[i1]));
        }

        static void DoQuery0D(Vector2<T> const& origin, Vector2<T> const& extent,
            Output& output)
        {
            output.parameter = C_<T>(0);
            output.closest[0] = origin;
            output.closest[1][0] = std::max(-extent[0], std::min(origin[0], extent[0]));
            output.closest[1][1] = std::max(-extent[1], std::min(origin[1], extent[1]));
        }
    };
}
