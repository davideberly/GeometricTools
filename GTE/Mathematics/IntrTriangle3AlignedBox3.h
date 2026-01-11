// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.1.2026.01.03

#pragma once

// The test-intersection query is based on the document
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection query clips the triangle against the faces of
// the oriented box.

#include <Mathematics/AlignedBox.h>
#include <Mathematics/IntrTriangle3CanonicalBox3.h>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Triangle3<T>, AlignedBox3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(Triangle3<T> const& triangle, AlignedBox3<T> const& box)
        {
            Result result{};

            // Transform the aligned box to a canonical box. Transform the
            // vertices accordingly.
            T const half = static_cast<T>(0.5);
            CanonicalBox3<T> canonicalBox = half * (box.max - box.min);
            Vector3<T> alignedBoxCenter = half * (box.max + box.min);

            Triangle3<T> transformedTriangle{};
            for (std::size_t i = 0; i < 3; ++i)
            {
                transformedTriangle.v[i] = triangle.v[i] - alignedBoxCenter;
            }

            TIQuery<T, Triangle3<T>, CanonicalBox3<T>> query{};
            result.intersect = query(transformedTriangle, canonicalBox).intersect;
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Triangle3<T>, AlignedBox3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                insidePolygon{},
                outsidePolygons{}
            {
            }

            std::vector<Vector3<T>> insidePolygon;
            std::vector<std::vector<Vector3<T>>> outsidePolygons;
        };

        Result operator()(Triangle3<T> const& triangle, AlignedBox3<T> const& box)
        {
            Result result{};

            // Start with the triangle and clip it against each face of the
            // box. The largest number of vertices for the polygon of
            // intersection is 7.
            result.insidePolygon.resize(3);
            for (std::int32_t i = 0; i < 3; ++i)
            {
                result.insidePolygon[i] = triangle.v[i];
            }

            // Create planes for the box faces that with normals that point
            // inside the box.
            T const half = static_cast<T>(0.5);
            Vector3<T> center = half * (box.max + box.min);
            Vector3<T> extent = half * (box.max - box.min);
            std::array<Plane3<T>, 6> planes{};
            planes[0].normal = -Vector3<T>::Unit(0);
            planes[0].constant = Dot(planes[0].normal, center) - extent[0];
            planes[1].normal = -Vector3<T>::Unit(1);
            planes[1].constant = Dot(planes[1].normal, center) - extent[1];
            planes[2].normal = -Vector3<T>::Unit(2);
            planes[2].constant = Dot(planes[2].normal, center) - extent[2];
            planes[3].normal = +Vector3<T>::Unit(0);
            planes[3].constant = Dot(planes[3].normal, center) - extent[0];
            planes[4].normal = +Vector3<T>::Unit(1);
            planes[4].constant = Dot(planes[4].normal, center) - extent[1];
            planes[5].normal = +Vector3<T>::Unit(2);
            planes[5].constant = Dot(planes[5].normal, center) - extent[2];

            for (auto const& plane : planes)
            {
                PPResult ppResult = PPQuery{}(result.insidePolygon, plane);
                switch (ppResult.configuration)
                {
                case PPQuery::Configuration::SPLIT:
                    result.insidePolygon = ppResult.positivePolygon;
                    result.outsidePolygons.push_back(ppResult.negativePolygon);
                    break;
                case PPQuery::Configuration::POSITIVE_SIDE_VERTEX:
                case PPQuery::Configuration::POSITIVE_SIDE_EDGE:
                case PPQuery::Configuration::POSITIVE_SIDE_STRICT:
                    // The result.insidePolygon is already
                    // ppResult.positivePolygon, but to make it clear,
                    // assign it here.
                    result.insidePolygon = ppResult.positivePolygon;
                    break;
                case PPQuery::Configuration::NEGATIVE_SIDE_VERTEX:
                case PPQuery::Configuration::NEGATIVE_SIDE_EDGE:
                case PPQuery::Configuration::NEGATIVE_SIDE_STRICT:
                    result.insidePolygon.clear();
                    result.outsidePolygons.push_back(ppResult.negativePolygon);
                    return result;
                case PPQuery::Configuration::CONTAINED:
                    // A triangle coplanar with a box face will be processed
                    // as if it were inside the box.
                    result.insidePolygon = ppResult.intersection;
                    break;
                default:
                    result.insidePolygon.clear();
                    result.outsidePolygons.clear();
                    break;
                }
            }

            return result;
        }

        using PPQuery = FIQuery<T, std::vector<Vector3<T>>, Plane3<T>>;
        using PPResult = typename PPQuery::Result;
    };
}

