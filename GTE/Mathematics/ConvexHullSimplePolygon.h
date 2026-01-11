// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

// Compute the convex hull of a simple polygon. The implementation is for
// the algorithm published in
//   On-line construction of the convex hull of a simple polyline
//   Avraham A. Melkman
//   Information Processing Letters 25 (1987), pages 11-12
//   North Holland Publishing Co.
// 
// A freely downloadable copy can be obtained by
//   https://www.ime.usp.br/~walterfm/cursos/mac0331/2006/melkman.pdf
// Elsevier acquired North Holland Publishing Co. and has made the PDF
// available via ScienceDirect
//   https://www.sciencedirect.com/science/article/pii/002001908790086X
// but you must pay for it (25 US dollars for personal or academic use,
// 35 US dollars for commercial use).
//
// A related webpage with a description of algorithm details is
//   https://cgm.cs.mcgill.ca/~athens/cs601/Melkman.html

#include <Mathematics/Logger.h>
#include <Mathematics/Vector2.h>
#include <algorithm>
#include <deque>
#include <numeric>
#include <vector>

namespace gte
{
    template <typename T>
    class ConvexHullSimplePolygon
    {
    public:
        // The polygon must be counterclockwise ordered, because the Minkowski
        // sum of convex polygon and disk assumes counterclockwise ordering.
        // The returned hull[] is an ordered list of indices into polygon[].
        // The hull vertices are
        //   { polygon[hull[0]], ..., polygon[hull[hull.size() - 1]] }
        // and the hull is counterclockwise ordered.

        void operator()(std::vector<Vector2<T>> const& polygon,
            std::vector<std::size_t>& hull)
        {
            std::size_t const n = polygon.size();
            LogAssert(
                n >= 3,
                "The input polygon must have at least 3 vertices."
            );

            // Melkman's algorithm converted to use C++ std::deque rather than
            // the double-ended queue in the pseudocode of the paper. Notice
            // that std::deque does not require the 'b' and 't' indices used
            // in the pseudocode of the paper.
            std::deque<std::size_t> dq{};
            if (WhichSide(polygon, 0, 1, 2) > 0)
            {
                dq.push_back(0);
                dq.push_back(1);
            }
            else
            {
                dq.push_back(1);
                dq.push_back(0);
            }
            dq.push_back(2);
            dq.push_front(2);

            std::size_t i = 2;
            while (++i < n)
            {
                // The incrementing of i in "++i < n" and the following
                // block of code are Step 2 of the PDF pseudocode. The
                // author's comment before the pseudocode is: "The algorithm
                // halts when its input is exhausted." It is unclear whether
                // the exhaustion occurs in "while (++i < n)" or in the
                // incrementing of i inside the while-loop below (or perhaps
                // it can occur in either based on the input polygon). Just
                // to be safe, range checking is performed after the
                // while-loop below terminates.
                while (
                    i < n &&
                    WhichSide(polygon, i, dq[0], dq[1]) >= 0 &&
                    WhichSide(polygon, dq[dq.size() - 2], dq[dq.size() - 1], i) >= 0)
                {
                    ++i;
                }
                if (i == n)
                {
                    break;
                }

                // This block of code is Step 3 of the PDF pseudocode.
                while (
                    WhichSide(polygon, dq[dq.size() - 2], dq[dq.size() - 1], i) <= 0)
                {
                    dq.pop_back();
                }
                dq.push_back(i);

                // This block of code is Step 4 of the PDF pseudocode.
                while (
                    WhichSide(polygon, i, dq[0], dq[1]) <= 0)
                {
                    dq.pop_front();
                }
                dq.push_front(i);
            }

            HullFromDoubleEndedQueue(dq, hull);
        }

    private:
        // Given directed edge <p0,p1>, determine which side of the line
        // of the directed edge contains the point p2. The function returns
        //   +1: p2 is on the right of the line
        //    0: p2 is on the line (p0, p1, and p2 are colinear)
        //   -1: p2 is on the left of the line
        std::int32_t WhichSide(
            std::vector<Vector2<T>> const& polygon,
            std::size_t i0, std::size_t i1, std::size_t i2)
        {
            T const zero = static_cast<T>(0);
            Vector2<T> diff10 = polygon[i1] - polygon[i0];
            Vector2<T> diff20 = polygon[i2] - polygon[i0];
            T test = DotPerp(diff20, diff10);
            return (test > zero ? +1 : (test < zero ? -1 : 0));
        }

        // Copy the double-ended queue into a std::vector container. The
        // input n is the number of vertices of the input polygon.
        void HullFromDoubleEndedQueue(std::deque<std::size_t> const& dq,
            std::vector<std::size_t>& hull)
        {
            // Guard against hull.resize(negativeNumber).
            LogAssert(
                dq.size() >= 2,
                "Invalid double-ended queue size."
            );

            std::size_t const lastIndex = dq.size() - 1;
            hull.resize(lastIndex);
            for (std::size_t i0 = 0, i1 = lastIndex; i0 < hull.size(); ++i0, --i1)
            {
                hull[i0] = dq[i1];
            }
        }
    };
}


