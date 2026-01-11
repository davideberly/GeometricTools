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

#include <Mathematics/OrientedBox.h>
#include <Mathematics/IntrTriangle3CanonicalBox3.h>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Triangle3<T>, OrientedBox3<T>>
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

        Result operator()(Triangle3<T> const& triangle, OrientedBox3<T> const& box)
        {
            Result result{};

            // Transform the oriented box to a canonical box. Transform the
            // triangle vertices accordingly.
            CanonicalBox3<T> canonicalBox(box.extent);

            Triangle3<T> transformedTriangle{};
            for (std::size_t j = 0; j < 3; ++j)
            {
                Vector3<T> diff = triangle.v[j] - box.center;
                for (std::int32_t i = 0; i < 3; ++i)
                {
                    transformedTriangle.v[j][i] = Dot(box.axis[i], diff);
                }
            }

            // Execute the test-intersection query.
            TCQuery tcQuery{};
            TCResult tcResult = tcQuery(transformedTriangle, canonicalBox);
            result.intersect = tcResult.intersect;

            return result;
        }

    private:
        using TCQuery = TIQuery<T, Triangle3<T>, CanonicalBox3<T>>;
        using TCResult = typename TCQuery::Result;
    };

    template <typename T>
    class FIQuery<T, Triangle3<T>, OrientedBox3<T>>
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

        Result operator()(Triangle3<T> const& triangle, OrientedBox3<T> const& box)
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
            std::array<Plane3<T>, 6> planes{};
            planes[0].normal = -box.axis[0];
            planes[0].constant = Dot(planes[0].normal, box.center) - box.extent[0];
            planes[1].normal = -box.axis[1];
            planes[1].constant = Dot(planes[1].normal, box.center) - box.extent[1];
            planes[2].normal = -box.axis[2];
            planes[2].constant = Dot(planes[2].normal, box.center) - box.extent[2];
            planes[3].normal = +box.axis[0];
            planes[3].constant = Dot(planes[3].normal, box.center) - box.extent[0];
            planes[4].normal = +box.axis[1];
            planes[4].constant = Dot(planes[4].normal, box.center) - box.extent[1];
            planes[5].normal = +box.axis[2];
            planes[5].constant = Dot(planes[5].normal, box.center) - box.extent[2];

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


