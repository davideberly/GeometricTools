// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The find-intersection query is based on the document
// https://www.geometrictools.com/Documentation/IntersectionMovingCircleRectangle.pdf

#include <Mathematics/IntrAlignedBox2Circle2.h>
#include <Mathematics/DistPointOrientedBox.h>
#include <array>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, OrientedBox2<T>, Circle2<T>>
    {
    public:
        // The intersection query considers the box and circle to be solids;
        // that is, the circle object includes the region inside the circular
        // boundary and the box object includes the region inside the
        // rectangular boundary.  If the circle object and rectangle object
        // overlap, the objects intersect.
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(OrientedBox2<T> const& box, Circle2<T> const& circle)
        {
            DCPQuery<T, Vector2<T>, OrientedBox2<T>> pbQuery;
            auto pbResult = pbQuery(circle.center, box);
            Result result{};
            result.intersect = (pbResult.sqrDistance <= circle.radius * circle.radius);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, OrientedBox2<T>, Circle2<T>>
        :
        public FIQuery<T, AlignedBox2<T>, Circle2<T>>
    {
    public:
        // See the base class for the definition of 'struct Result'.
        typename FIQuery<T, AlignedBox2<T>, Circle2<T>>::Result
        operator()(OrientedBox2<T> const& box, Vector2<T> const& boxVelocity,
            Circle2<T> const& circle, Vector2<T> const& circleVelocity)
        {
            // Transform the oriented box to an axis-aligned box centered at
            // the origin and transform the circle accordingly.  Compute the
            // velocity of the circle relative to the box.
            T const zero(0), one(1), minusOne(-1);
            Vector2<T> cdiff = circle.center - box.center;
            Vector2<T> vdiff = circleVelocity - boxVelocity;
            Vector2<T> C, V;
            for (int32_t i = 0; i < 2; ++i)
            {
                C[i] = Dot(cdiff, box.axis[i]);
                V[i] = Dot(vdiff, box.axis[i]);
            }

            // Change signs on components, if necessary, to transform C to the
            // first quadrant.  Adjust the velocity accordingly.
            std::array<T, 2> sign{ (T)0, (T)0 };
            for (int32_t i = 0; i < 2; ++i)
            {
                if (C[i] >= zero)
                {
                    sign[i] = one;
                }
                else
                {
                    C[i] = -C[i];
                    V[i] = -V[i];
                    sign[i] = minusOne;
                }
            }

            typename FIQuery<T, AlignedBox2<T>, Circle2<T>>::Result result{};
            this->DoQuery(box.extent, C, circle.radius, V, result);

            if (result.intersectionType != 0)
            {
                // Transform back to the original coordinate system.
                result.contactPoint = box.center
                    + (sign[0] * result.contactPoint[0]) * box.axis[0]
                    + (sign[1] * result.contactPoint[1]) * box.axis[1];
            }
            return result;
        }
    };
}
