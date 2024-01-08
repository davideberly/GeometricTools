// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

// The polyline has N vertices. If the polyline is open (with N >= 2), the
// segments are <V[0],V[1]>, <V[1],V[2]>, ..., <V[N-2],V[N-1]>. If the
// polyline is closed (with N >= 3), the segments are those of the open
// polyline and the segment <P[N-1],P[0]>. The geometry of the polyline is not
// taken into account in the algorithm. For example, the algorithm does not
// test whether the segments intersect at interior points. If you want an
// offset for a simple polygon, you must ensure that the incoming points form
// a simple polygon. The offset itself might not be a simple polygon when the
// offset distance is sufficiently large.
//
// The segment <V[i], V[i + 1]> is directed with unit-length direction
//   D = (V[i + 1] - V[i])/|V[i + 1] - V[i]|
// A unit-length normal to the segment is chosen to point to the right of the
// segment,
//   N = Perp(D)
// where Perp(x,y) = (y,-x).
//
// For 3 consecutive vertices <V[i], V[i + 1], V[i + 2]>, it is allowed that
// directed segments <V[i], V[i + 1]> and <V[i + 1], V[i + 2]> be parallel as
// long as the direction vectors are in the same direction. That is, if D[i]
// is the direction of the first segment and D[i + 1] is the direction of the
// second segment, then Dot(D[i], D[i + 1]) = 1. It is not allowed that the
// direction vectors are in the opposite direction. That is, it is not allowed
// that Dot(Dot(D[i], D[i + 1]) = -1. In this situation, there is a
// singularity in the offset distance at D[i + 1].
//
// To compute the offset polyline in the positive normal direction (offset is
// to the right of segments), set 'offsetRight' to 'true'. The offset polyline
// is returned as 'rightPolyline'. To compute the offset polyline in the
// negative normal direction (offset is to the left of segments), set
// 'offsetLeft' to 'true'. The offset polyline is returned as 'leftPolyline.'
// You can set both Boolean values to 'true' when you want both polylines.
//
// NOTE: The offset depends on the geometry of the polyline. As the offset
// distance increases, the offset polylines can "fold over". The visualization
// will not look right. This code makes no attempt to determine a maximum
// offset distance for which fold-over occurs once you exceed that maximum.

#include <Mathematics/Logger.h>
#include <Mathematics/Vector2.h>
#include <cstddef>
#include <vector>

namespace gte
{
    template <typename T>
    class PolylineOffset
    {
    public:
        PolylineOffset(std::vector<Vector2<T>> const& vertices, bool isOpen)
            :
            mVertices(vertices),
            mIsOpen(isOpen),
            mDirections(isOpen ? vertices.size() - 1 : vertices.size()),
            mNormals(isOpen ? vertices.size() - 1 : vertices.size())
        {
            size_t const numVertices = mVertices.size();
            LogAssert(
                numVertices >= (mIsOpen ? 2 : 3),
                "Invalid number of polyline vertices.");

            mDirections.resize(mIsOpen ? numVertices - 1 : numVertices);
            mNormals.resize(mIsOpen ? numVertices - 1 : numVertices);

            for (size_t i0 = 0, i1 = 1; i1 < numVertices; i0 = i1++)
            {
                auto const& V0 = mVertices[i0];
                auto const& V1 = mVertices[i1];
                auto& D0 = mDirections[i0];
                auto& N0 = mNormals[i0];
                D0 = V1 - V0;
                Normalize(D0);
                N0 = Perp(D0);
            }

            if (!mIsOpen)
            {
                size_t const numVerticesM1 = numVertices - 1;
                auto const& V0 = mVertices[numVerticesM1];
                auto const& V1 = mVertices[0];
                auto& D0 = mDirections[numVerticesM1];
                auto& N0 = mNormals[numVerticesM1];
                D0 = V1 - V0;
                Normalize(D0);
                N0 = Perp(D0);
            }
        }

        // Disallow copying and moving because the incoming vertices are
        // stored as a const reference member.
        PolylineOffset(PolylineOffset const&) = delete;
        PolylineOffset& operator=(PolylineOffset const&) = delete;
        PolylineOffset(PolylineOffset&&) = delete;
        PolylineOffset& operator=(PolylineOffset&&) = delete;

        void Execute(T const& offsetDistance,
            bool offsetRight, std::vector<Vector2<T>>& rightPolyline,
            bool offsetLeft, std::vector<Vector2<T>>& leftPolyline)
        {
            LogAssert(
                offsetDistance > static_cast<T>(0),
                "The offset distance must be positive.");

            LogAssert(
                offsetRight || offsetLeft,
                "Expecting a directive to compute an offset polyline.");

            if (offsetRight)
            {
                ComputeRightPolyline(offsetDistance, rightPolyline);
            }

            if (offsetLeft)
            {
                ComputeLeftPolyline(offsetDistance, leftPolyline);
            }
        }

    private:
        void ComputeRightPolyline(T const& distance, std::vector<Vector2<T>>& polyline)
        {
            size_t const numVertices = mVertices.size();
            polyline.reserve(numVertices);
            polyline.clear();

            // Process the first endpoint depending on whether the polyline is
            // open or closed.
            T const one = static_cast<T>(1);
            if (mIsOpen)
            {
                auto const& V0 = mVertices.front();
                auto const& N0 = mNormals.front();
                polyline.push_back(V0 + distance * N0);
            }
            else
            {
                auto const& V1 = mVertices.front();
                auto const& N0 = mNormals.back();
                auto const& N1 = mNormals.front();
                polyline.push_back(V1 + (distance / (one + Dot(N0, N1))) * (N0 + N1));
            }

            // B = N[i0] + N[i1] is the bisector direction at V[i1]] for the
            // two normals of the edges sharing V[i1]. The offset vertex is
            // V[i1] + (d / Dot(N[i0],B)) * B, where d is the segment offset
            // distance. B does not have to be normalized because the offset
            // vertex is independent of length of B. The offset vertex is
            // therefore
            //   V[i1] + (d / (1 + Dot(N[i0], N[i1]) * (N[i0] + N[i1])
            for (size_t i0 = 0, i1 = 1, i2 = 2; i2 < numVertices; i0 = i1, i1 = i2++)
            {
                auto const& V1 = mVertices[i1];
                auto const& N0 = mNormals[i0];
                auto const& N1 = mNormals[i1];
                polyline.push_back(V1 + (distance / (one + Dot(N0, N1))) * (N0 + N1));
            }

            // Process the last endpoint depending on whether the polyline is
            // open or closed.
            if (mIsOpen)
            {
                auto const& V0 = mVertices.back();
                auto const& N0 = mNormals.back();
                polyline.push_back(V0 + distance * N0);
            }
            else
            {
                auto const& V1 = mVertices.back();
                auto const& N0 = mNormals[numVertices - 2];
                auto const& N1 = mNormals.back();
                polyline.push_back(V1 + (distance / (one + Dot(N0, N1))) * (N0 + N1));
            }
        }

        void ComputeLeftPolyline(T const& distance, std::vector<Vector2<T>>& polyline)
        {
            size_t const numVertices = mVertices.size();
            polyline.reserve(numVertices);
            polyline.clear();

            // Process the first endpoint depending on whether the polyline is
            // open or closed.
            T const one = static_cast<T>(1);
            if (mIsOpen)
            {
                auto const& V0 = mVertices.front();
                auto const& N0 = mNormals.front();
                polyline.push_back(V0 - distance * N0);
            }
            else
            {
                auto const& V1 = mVertices.front();
                auto const& N0 = mNormals.back();
                auto const& N1 = mNormals.front();
                polyline.push_back(V1 - (distance / (one + Dot(N0, N1))) * (N0 + N1));
            }

            // B = N[i0] + N[i1] is the bisector direction at V[i1]] for the
            // two normals of the edges sharing V[i1]. The offset vertex is
            // V[i1] + (d / Dot(N[i0],B)) * B, where d is the segment offset
            // distance. B does not have to be normalized because the offset
            // vertex is independent of length of B. The offset vertex is
            // therefore
            //   V[i1] + (d / (1 + Dot(N[i0], N[i1]) * (N[i0] + N[i1])
            for (size_t i0 = 0, i1 = 1, i2 = 2; i2 < numVertices; i0 = i1, i1 = i2++)
            {
                auto const& V1 = mVertices[i1];
                auto const& N0 = mNormals[i0];
                auto const& N1 = mNormals[i1];
                polyline.push_back(V1 - (distance / (one + Dot(N0, N1))) * (N0 + N1));
            }

            // Process the last endpoint depending on whether the polyline is
            // open or closed.
            if (mIsOpen)
            {
                auto const& V0 = mVertices.back();
                auto const& N0 = mNormals.back();
                polyline.push_back(V0 - distance * N0);
            }
            else
            {
                auto const& V1 = mVertices.back();
                auto const& N0 = mNormals[numVertices - 2];
                auto const& N1 = mNormals.back();
                polyline.push_back(V1 - (distance / (one + Dot(N0, N1))) * (N0 + N1));
            }
        }

        std::vector<Vector2<T>> const& mVertices;
        bool mIsOpen;
        std::vector<Vector2<T>> mDirections;
        std::vector<Vector2<T>> mNormals;
    };
}
