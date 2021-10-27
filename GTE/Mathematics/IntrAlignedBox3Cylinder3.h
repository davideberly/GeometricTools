// David Eberly, Geometric Tools, Redmond WA 98052
// Sebastian Wouters
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/Cylinder3.h>

// The query considers the cylinder and box to be solids.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, AlignedBox3<Real>, Cylinder3<Real>>
    {
    public:

        struct Result
        {
            bool intersect;
        };

    private:

        template <size_t size>
        uint8_t convexHull2dSimple(std::array<uint8_t, size>& hull, const uint8_t numPoints, const std::array<Vector<2, Real>, size>& points) const
        {
            if (numPoints <= 2)
            {
                for (uint8_t idx = 0; idx < numPoints; ++idx) { hull[idx] = idx; }
                return numPoints;
            }

            // Compute the convex hull counter clockwise
            // Use Andrew's algorithm (see section 3.9.1 of Christer, Real-time collision detection (2005), ISBN 1-55860-732-3)
            std::array<uint8_t, size> sorted = {};
            for (uint8_t idx = 0; idx < numPoints; ++idx) { sorted[idx] = idx; }
            std::sort(sorted.begin(), sorted.begin() + numPoints,
                [&points](const uint8_t left, const uint8_t right)
                {
                    return (points[left][0] <  points[right][0]) ||
                          ((points[left][0] == points[right][0]) && (points[left][1] < points[right][1]));
                });

            const uint8_t jMin = sorted[0];
            const uint8_t jMax = sorted[numPoints - 1];

            // Lower half: points below line(points[jMin], points[jMax])
            hull[0] = jMin;
            uint8_t last = 0;
            for (uint8_t next = 1; next < numPoints - 1; ++next)
            {
                const uint8_t jNext = sorted[next];
                const uint8_t jLast = hull[last];
                Vector<2, Real> v1 = points[jMax]  - points[jLast];
                Vector<2, Real> v2 = points[jNext] - points[jLast];
                // if points[jNext] below line(points[jLast], points[jMax]): insert jNext in hull
                if (v1[0] * v2[1] < v1[1] * v2[0])
                {
                    // points[jMin] must belong to hull: do not check erasing jMin
                    bool check = last != 0;
                    while (check)
                    {
                        const uint8_t j1 = hull[last];
                        const uint8_t j0 = hull[last - 1];
                        v1 = points[j1]    - points[j0];
                        v2 = points[jNext] - points[j0];
                        // if points[jNext] below or on line(points[j0], points[j1]): erase j1 from hull
                        check = (v1[0] * v2[1] <= v1[1] * v2[0]) && (--last != 0);
                    }
                    hull[++last] = jNext;
                    // if points[jNext] below line(points[jMin], points[jMax]), skip check whether above:
                    sorted[next] = UINT8_MAX;
                }
            }

            // Upper half: points above line(points[jMin], points[jMax])
            hull[++last] = jMax;
            const uint8_t limit = last;
            for (uint8_t next = numPoints - 2; next > 0; --next)
            {
                const uint8_t jNext = sorted[next];
                if (jNext != UINT8_MAX)
                {
                    const uint8_t jLast = hull[last];
                    Vector<2, Real> v1 = points[jMin]  - points[jLast];
                    Vector<2, Real> v2 = points[jNext] - points[jLast];
                    // if points[jNext] above line(points[jLast], points[jMin]): insert jNext in hull
                    if (v1[0] * v2[1] < v1[1] * v2[0])
                    {
                        // points[jMax] must belong to hull: do not check erasing jMax
                        bool check = last != limit;
                        while (check)
                        {
                            const uint8_t j1 = hull[last];
                            const uint8_t j0 = hull[last - 1];
                            v1 = points[j1]    - points[j0];
                            v2 = points[jNext] - points[j0];
                            // if points[jNext] above or on line(points[j0], points[j1]): erase j1 from hull
                            check = (v1[0] * v2[1] <= v1[1] * v2[0]) && (--last != limit);
                        }
                        hull[++last] = jNext;
                    }
                }
            }

            return ++last;
        }

    public:

        Result operator()(AlignedBox3<Real> const& box, Cylinder3<Real> const& cyl) const
        {
            constexpr Result disjoint   = { false };
            constexpr Result intersects = { true  };

            struct Vertex
            {
                const Vector<3, Real> coord;
                const Real            proj;
                Vertex(const std::array<Real, 3>& in, const Vector<3, Real>& axis)
                    : coord(in)
                    , proj(Dot(coord, axis)) {}
            };

            /*
                z
                ^  6-----7
                | /|    /|
                |/ |   / |
                4--+--5  |
                |  2--+--3
                | /   | /
                |/    |/
                0-----1----> x
            */

            const std::array<Vertex, 8> vertices = {
                Vertex({ box.min[0], box.min[1], box.min[2] }, cyl.axis.direction),
                Vertex({ box.max[0], box.min[1], box.min[2] }, cyl.axis.direction),
                Vertex({ box.min[0], box.max[1], box.min[2] }, cyl.axis.direction),
                Vertex({ box.max[0], box.max[1], box.min[2] }, cyl.axis.direction),
                Vertex({ box.min[0], box.min[1], box.max[2] }, cyl.axis.direction),
                Vertex({ box.max[0], box.min[1], box.max[2] }, cyl.axis.direction),
                Vertex({ box.min[0], box.max[1], box.max[2] }, cyl.axis.direction),
                Vertex({ box.max[0], box.max[1], box.max[2] }, cyl.axis.direction) };

            const Real midAxisCyl = Dot(cyl.axis.direction, cyl.axis.origin);
            const Real halfHeight = static_cast<Real>(0.5) * cyl.height;
            const Real maxAxisCyl = midAxisCyl + halfHeight;
            const Real minAxisCyl = midAxisCyl - halfHeight;

            bool allAbove = true;
            bool allBelow = true;
            for (const Vertex& vtx : vertices)
            {
                allAbove = allAbove && (vtx.proj > maxAxisCyl);
                allBelow = allBelow && (vtx.proj < minAxisCyl);
            }
            if (allAbove || allBelow)
                return disjoint; // cyl.axis.direction is a separating axis

            // Compute the vertices of a polyhedron which is the box capped by the cylinder's
            // top and bottom planes and project them into a plane perpendicular to the
            // cylinder axis direction.
            const Vector<3, Real> direction1 = fabs(cyl.axis.direction[0]) > static_cast<Real>(0.9)
                ? UnitCross({ static_cast<Real>(0.0), static_cast<Real>(1.0), static_cast<Real>(0.0) }, cyl.axis.direction)
                : UnitCross({ static_cast<Real>(1.0), static_cast<Real>(0.0), static_cast<Real>(0.0) }, cyl.axis.direction);
            const Vector<3, Real> direction2 = Cross(cyl.axis.direction, direction1);

            // Suppose N vertices are cropped by the cyl's top plane:
            //      no. of vertices convex polyhedron
            //          <= 8         [original]
            //           - N         [cropped]
            //           + 3 * N     [cropped vertex splits in 3 along its edges due to crop plane]
            //           - 2 * (N-1) [*]
            //          <= 8 [original] + 2 [independent of N]
            // [*] After a first vertex is cropped by a crop plane, each additional vertex cropped
            //     by that crop plane is attached via one or more of its edges to one or more of
            //     the already cropped vertices (cropped by that crop plane). The splits accounted
            //     for by the term 3 * N are hence corrected by this term, as edges in between two
            //     vertices cropped by that crop plane are removed as well by that crop plane.
            // Same consideration for cyl's bottom plane: total no. of vertices <= 8 + 2 + 2 = 12.
            std::array<Vector<2, Real>, 12> points;
            int numPoints = 0;

            for (uint32_t idx = 0; idx < vertices.size(); ++idx)
            {
                const Vertex& vtx = vertices[idx];
                if (vtx.proj > maxAxisCyl)
                {
                    for (uint32_t shift = 0; shift < 3; ++shift)
                    {
                        const Vertex& neighbor = vertices[idx ^ (1U << shift)];
                        if (neighbor.proj > maxAxisCyl)
                            continue; // both cropped by maxAxisCyl
                        const Real denom = vtx.proj - neighbor.proj; // denom > 0.0
                        if (denom == static_cast<Real>(0.0))
                            continue; // floating point accuracy check
                        const Real z = (maxAxisCyl - neighbor.proj) / denom; // 1.0 > z >= 0.0
                        const Vector<3, Real> point = z * vtx.coord + (static_cast<Real>(1.0) - z) * neighbor.coord;
                        points[numPoints++] = { Dot(direction1, point), Dot(direction2, point) };
                    }
                }
                else if (vtx.proj < minAxisCyl)
                {
                    for (uint32_t shift = 0; shift < 3; ++shift)
                    {
                        const Vertex& neighbor = vertices[idx ^ (1U << shift)];
                        if (neighbor.proj < minAxisCyl)
                            continue; // both cropped by minAxisCyl
                        const Real denom = vtx.proj - neighbor.proj; // denom < 0.0
                        if (denom == static_cast<Real>(0.0))
                            continue; // floating point accuracy check
                        const Real z = (minAxisCyl - neighbor.proj) / denom; // 1.0 > z >= 0.0
                        const Vector<3, Real> point = z * vtx.coord + (static_cast<Real>(1.0) - z) * neighbor.coord;
                        points[numPoints++] = { Dot(direction1, point), Dot(direction2, point) };
                    }
                }
                else
                {
                    points[numPoints++] = { Dot(direction1, vtx.coord), Dot(direction2, vtx.coord) };
                }
            }

            // The polyhedron is convex. Its projection is a convex polygon.
            // The convex polygon can be computed as the convex hull of the projected polyhedron's vertices.
            // Determine whether the polygon contains a point sufficiently close to the cylinder's center.
            const Vector<2, Real> center = { Dot(direction1, cyl.axis.origin), Dot(direction2, cyl.axis.origin) };
            const Real radiusSquared = cyl.radius * cyl.radius;
            if (numPoints == 0)
                return disjoint;
            else if (numPoints == 1)
            {
                Vector<2, Real> diff = points[0] - center;
                return Dot(diff, diff) > radiusSquared ? disjoint : intersects;
            }

            std::array<uint8_t, points.size()> hull;
            const uint8_t end = convexHull2dSimple(hull, numPoints, points);

            // Compare center's distance w.r.t. hull and/or whether center is enclosed in the hull
            bool enclosed = true;
            for (uint8_t idx = 0; idx < end; ++idx)
            {
                const Vector<2, Real>& P = points[hull[idx]];
                const Vector<2, Real>& Q = points[hull[(idx + 1) % end]];
                const Vector<2, Real> PQ = Q - P;
                const Vector<2, Real> PC = center - P;
                if (PQ[0] * PC[1] < PQ[1] * PC[0])
                    enclosed = false;
                // point(z) = z * Q + (1 - z) * P
                // |point(z) - C|^2 = |z * PQ - PC|^2
                const Real PQsq = Dot(PQ, PQ);
                const Real PCsq = Dot(PC, PC);
                Real value = static_cast<Real>(0.0);
                if (PQsq == static_cast<Real>(0.0))
                    value = PCsq;
                else
                {
                    const Real PQdotPC = Dot(PQ, PC);
                    const Real z = PQdotPC / PQsq;
                    if (z <= static_cast<Real>(0.0))
                        value = PCsq;
                    else if (z >= static_cast<Real>(1.0))
                    {
                        const Vector<2, Real> QC = center - Q;
                        value = Dot(QC, QC);
                    }
                    else
                        value = z * (z * PQsq - static_cast<Real>(2.0) * PQdotPC) + PCsq;
                }
                if (value <= radiusSquared)
                    return intersects;
            }
            if (enclosed)
                return intersects;
            else
                return disjoint;
        }
    };
}

