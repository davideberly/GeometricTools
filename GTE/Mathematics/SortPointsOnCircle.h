// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

// The sorting algorithms are described in
// https://www.geometrictools.com/Documentation/SortPointsOnCircle.pdf
// The input P[] are points and C is the center point about which the points
// are to be sorted. The reference ray (angle 0) is C+t*D for initial point
// C and nonzero direction D. The direction does not have to be unit length.
// If sortCCW is 'true', the angles counterclockwise from the reference ray
// are positive and in [0,pi]. The angles clockwise from the reference ray
// are negative and in (-pi,0]. If sortCCW is 'false', the angles clockwise
// from the reference ray are positive and in [0,pi]. The angles
// counterclockwise from the reference ray are negative and in (-pi,0]. The
//  output 'indices[]' provides an indirect sorting. The sorted points are
// P[indices[0]], P[indices[1]], ..., P[indices[P.size()-1]].

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace gte
{
    template <typename T>
    class SortPointsOnCircle
    {
    public:
        // The sorting algorithm uses std::atan2 and contains arithmetic
        // operations, all subject to floating-point rounding errors when
        // T is 'float' or 'double'. An exact rational type for T does not
        // fix the problem because std::atan2 has mathematical errors because
        // the function cannot be implemented to produce exact angles using
        // only arithemtic operations.
        static void ByAngle(std::vector<std::array<T, 2>> const& P,
            std::array<T, 2> const& C, std::array<T, 2> const& D, bool sortCCW,
            std::vector<size_t>& indices)
        {
            std::array<T, 2> Dperp = (sortCCW ? std::array<T, 2>{ -D[1], D[0] } : std::array<T, 2>{ D[1], -D[0] });
            std::vector<SortObject> object(P.size());
            for (size_t i = 0; i < P.size(); ++i)
            {
                std::array<T, 2> V = { P[i][0] - C[0], P[i][1] - C[1] };
                object[i].W = { D[0] * V[0] + D[1] * V[1], Dperp[0] * V[0] + Dperp[1] * V[1] };
                object[i].index = i;
            }
            std::sort(object.begin(), object.end(), LessThanByAngle);

            indices.resize(P.size());
            for (size_t i = 0; i < P.size(); ++i)
            {
                indices[i] = object[i].index;
            }
        }

        // The sorting algorithm uses only arithmetic operations. It supports
        // T of 'float' or 'double' but the correctness is not guaranteed to
        // be theoretically correct because of rounding errors. If T is an
        // exact rational type, the output is theoretically correct.
        static void ByGeometry(std::vector<std::array<T, 2>> const& P,
            std::array<T, 2> const& C, std::array<T, 2> const& D, bool sortCCW,
            std::vector<size_t>& indices)
        {
            std::array<T, 2> Dperp = (sortCCW ? std::array<T, 2>{ -D[1], D[0] } : std::array<T, 2>{ D[1], -D[0] });
            std::vector<SortObject> object(P.size());
            for (size_t i = 0; i < P.size(); ++i)
            {
                std::array<T, 2> V = { P[i][0] - C[0], P[i][1] - C[1] };
                object[i].W = { D[0] * V[0] + D[1] * V[1], Dperp[0] * V[0] + Dperp[1] * V[1] };
                object[i].index = i;
            }
            std::sort(object.begin(), object.end(), LessThanByGeometry);

            indices.resize(P.size());
            for (size_t i = 0; i < P.size(); ++i)
            {
                indices[i] = object[i].index;
            }
        }

    private:
        struct SortObject
        {
            SortObject()
                :
                W{ static_cast<T>(0), static_cast<T>(0) },
                index(0)
            {
            }

            std::array<T, 2> W;
            size_t index;
        };

        static bool LessThanByAngle(SortObject const& object0, SortObject const& object1)
        {
            T const& x0 = object0.W[0], y0 = object0.W[1];
            T const& x1 = object1.W[0], y1 = object1.W[1];

            T angle0 = std::atan2(y0, x0);
            T angle1 = std::atan2(y1, x1);
            if (angle0 < angle1)
            {
                return true;
            }
            if (angle0 > angle1)
            {
                return false;
            }

            return (x0 - x1) * (x0 + x1) < (y1 - y0) * (y1 + y0);  // c == 0, s0 < s1
        }

        static bool LessThanByGeometry(SortObject const& object0, SortObject const& object1)
        {
            T const& x0 = object0.W[0], y0 = object0.W[1];
            T const& x1 = object1.W[0], y1 = object1.W[1];
            T const zero = static_cast<T>(0);

            if (y0 < zero&& y1 >= zero)
            {
                return true;
            }

            if (y1 < zero && y0 >= zero)
            {
                return false;
            }

            if (y0 > zero && y1 == zero)
            {
                return x1 < zero;
            }

            if (y1 > zero && y0 == zero)
            {
                return x0 > zero;
            }

            if (y0 == zero && y1 == zero)
            {
                return (x1 < zero && x1 < x0) || (x0 > zero && x1 > x0);
            }

            T c = x0 * y1 - x1 * y0;
            if (c > zero)
            {
                return true;
            }

            if (c < zero)
            {
                return false;
            }

            return (x0 - x1) * (x0 + x1) < (y1 - y0) * (y1 + y0);  // c == 0, s0 < s1
        }
    };
}
