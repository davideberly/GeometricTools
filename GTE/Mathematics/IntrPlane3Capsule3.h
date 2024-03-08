// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/DistPointHyperplane.h>
#include <Mathematics/Capsule.h>
#include <cmath>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Plane3<Real>, Capsule3<Real>>
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

        Result operator()(Plane3<Real> const& plane, Capsule3<Real> const& capsule)
        {
            Result result{};

            DCPQuery<Real, Vector3<Real>, Plane3<Real>> vpQuery;
            Real sdistance0 = vpQuery(capsule.segment.p[0], plane).signedDistance;
            Real sdistance1 = vpQuery(capsule.segment.p[1], plane).signedDistance;
            if (sdistance0 * sdistance1 <= (Real)0)
            {
                // A capsule segment endpoint is on the plane or the two
                // endpoints are on opposite sides of the plane.
                result.intersect = true;
                return result;
            }

            // The endpoints on same side of plane, but the endpoint spheres
            // might intersect the plane.
            result.intersect =
                std::fabs(sdistance0) <= capsule.radius ||
                std::fabs(sdistance1) <= capsule.radius;
            return result;
        }
    };
}
