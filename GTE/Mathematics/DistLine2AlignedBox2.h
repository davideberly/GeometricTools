// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

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

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Functions.h>
#include <Mathematics/Line.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/OrientedBox.h>
#include <Mathematics/Vector2.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Line2<T>, AlignedBox2<T>>
    {
    public:
        struct Result
        {

            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                parameter(static_cast<T>(0)),
                closest{ {
                    { static_cast<T>(0), static_cast<T>(0) },
                    { static_cast<T>(0), static_cast<T>(0) }
                } }
            {
            }

            T distance, sqrDistance;
            T parameter;
            std::array<Vector2<T>, 2> closest;
        };

        Result operator()(Line2<T> const& line, AlignedBox2<T> const& box)
        {
            Result result{};

            // Translate the line and box so that the box has center at the
            // origin.
            Vector2<T> boxCenter{}, boxExtent{};
            box.GetCenteredForm(boxCenter, boxExtent);
            Vector2<T> origin = line.origin - boxCenter;
            Vector2<T> direction = line.direction;

            // The query computes 'result' relative to the box with center
            // at the origin.
            DoQuery(origin, direction, boxExtent, result);

            // Translate the closest points to the original coordinates.
            for (size_t i = 0; i < 2; ++i)
            {
                result.closest[i] += boxCenter;
            }

            // Compute the distance and squared distance.
            Vector2<T> diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
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
            Vector2<T> const& extent, Result& result)
        {
            // Apply reflections so that the direction has nonnegative
            // components.
            T const zero = static_cast<T>(0);
            std::array<bool, 2> reflect{ false, false };
            for (int32_t i = 0; i < 2; ++i)
            {
                if (direction[i] < zero)
                {
                    origin[i] = -origin[i];
                    direction[i] = -direction[i];
                    reflect[i] = true;
                }
            }

            // Compute the line-box distance and closest points. The DoQueryND
            // calls compute result.parameter and result.closest[1]. The
            // result.closest[0] can be computed after these calls.
            if (direction[0] > zero)
            {
                if (direction[1] > zero)
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
                    DoQuery2D(origin, direction, extent, result);
                }
                else
                {
                    // The direction signs are (+,0). The parameter is the
                    // value of t for which P + t * D = (e0, p1).
                    DoQuery1D(0, 1, origin, direction, extent, result);
                }
            }
            else
            {
                if (direction[1] > zero)
                {
                    // The direction signs are (0,+). The parameter is the
                    // value of t for which P + t * D = (p0, e1).
                    DoQuery1D(1, 0, origin, direction, extent, result);
                }
                else
                {
                    // The direction signs are (0,0). The line is degenerate
                    // to a point (its origin). Clamp the origin to the box
                    // to obtain the closest point.
                    DoQuery0D(origin, extent, result);
                }
            }

            result.closest[0] = origin + result.parameter * direction;

            // Undo the reflections. The origin and direction are not consumed
            // by the caller, so these do not need to be reflected. However,
            // the closest points are consumed.
            for (int32_t i = 0; i < 2; ++i)
            {
                if (reflect[i])
                {
                    for (size_t j = 0; j < 2; ++j)
                    {
                        result.closest[j][i] = -result.closest[j][i];
                    }
                }
            }
        }

    private:
        // Allow the line-oriented_box query to access DoQuery without having
        // to use class derivation.
        friend class DCPQuery<T, Line2<T>, OrientedBox2<T>>;

        static void DoQuery2D(Vector2<T> const& origin, Vector2<T> const& direction,
            Vector2<T> const& extent, Result& result)
        {
            T const zero = static_cast<T>(0);
            Vector2<T> K0{ -extent[0], extent[1] };
            Vector2<T> delta = K0 - origin;
            T K0dotPerpD = DotPerp(delta, direction);
            if (K0dotPerpD >= zero)
            {
                result.parameter = Dot(delta, direction) / Dot(direction, direction);
                result.closest[0] = origin + result.parameter * direction;
                result.closest[1] = K0;
            }
            else
            {
                Vector2<T> K1{ extent[0], -extent[1] };
                delta = K1 - origin;
                T K1dotPerpD = DotPerp(delta, direction);
                if (K1dotPerpD <= zero)
                {
                    result.parameter = Dot(delta, direction) / Dot(direction, direction);
                    result.closest[0] = origin + result.parameter * direction;
                    result.closest[1] = K1;
                }
                else
                {
                    Vector2<T> K2{ extent[0], extent[1] };
                    delta = K2 - origin;
                    T K2dotPerpD = DotPerp(delta, direction);
                    if (K2dotPerpD >= zero)
                    {
                        result.parameter = (extent[1] - origin[1]) / direction[1];
                        result.closest[0] = origin + result.parameter * direction;
                        result.closest[1][0] = origin[0] + result.parameter * direction[0];
                        result.closest[1][1] = extent[1];
                    }
                    else
                    {
                        result.parameter = (extent[0] - origin[0]) / direction[0];
                        result.closest[0] = origin + result.parameter * direction;
                        result.closest[1][0] = extent[0];
                        result.closest[1][1] = origin[1] + result.parameter * direction[1];
                    }
                }
            }
        }

        static void DoQuery1D(int32_t i0, int32_t i1, Vector2<T> const& origin,
            Vector2<T> const& direction, Vector2<T> const& extent, Result& result)
        {
            result.parameter = (extent[i0] - origin[i0]) / direction[i0];
            result.closest[0] = origin + result.parameter * direction;
            result.closest[1][i0] = extent[i0];
            result.closest[1][i1] = clamp(origin[i1], -extent[i1], extent[i1]);
        }

        static void DoQuery0D(Vector2<T> const& origin, Vector2<T> const& extent,
            Result& result)
        {
            result.parameter = static_cast<T>(0);
            result.closest[0] = origin;
            result.closest[1][0] = clamp(origin[0], -extent[0], extent[0]);
            result.closest[1][1] = clamp(origin[1], -extent[1], extent[1]);
        }
    };
}
