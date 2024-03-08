// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the box and cone to be solids.
//
// Define V = cone.ray.origin, D = cone.ray.direction, and cs = cone.cosAngle.
// Define C = box.center, U0 = box.axis[0], U1 = box.axis[1],
// e0 = box.extent[0], and e1 = box.extent[1].  A box point is
// P = C + x*U0 + y*U1 where |x| <= e0 and |y| <= e1.  Define the function
//   F(P) = Dot(D, (P-V)/Length(P-V)) = F(x,y)
//     = Dot(D, (x*U0 + y*U1 + (C-V))/|x*U0 + y*U1 + (C-V)|
//     = (a0*x + a1*y + a2)/(x^2 + y^2 + 2*b0*x + 2*b1*y + b2)^{1/2}
// The function has an essential singularity when P = V.  The box intersects
// the cone (with positive-area overlap) when at least one of the four box
// corners is strictly inside the cone.  It is necessary that the numerator
// of F(P) be positive at such a corner.  The (interior of the) solid cone
// is defined by the quadratic inequality
//   (Dot(D,P-V))^2 > |P-V|^2*(cone.cosAngle)^2
// This inequality is inexpensive to compute.  In summary, overlap occurs
// when there is a box corner P for which
//   F(P) > 0 and (Dot(D,P-V))^2 > |P-V|^2*(cone.cosAngle)^2

#include <Mathematics/IntrRay2OrientedBox2.h>
#include <Mathematics/Cone.h>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, OrientedBox<2, T>, Cone<2, T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            // The value of 'intersect' is true when there is a box point that
            // is strictly inside the cone.  If the box just touches the cone
            // from the outside, an intersection is not reported, which
            // supports the common operation of culling objects outside a
            // cone.
            bool intersect;
        };

        Result operator()(OrientedBox<2, T> const& box, Cone<2, T>& cone)
        {
            Result result{};

            TIQuery<T, Ray<2, T>, OrientedBox<2, T>> rbQuery;
            auto rbResult = rbQuery(cone.ray, box);
            if (rbResult.intersect)
            {
                // The cone intersects the box.
                result.intersect = true;
                return result;
            }

            // Define V = cone.ray.origin, D = cone.ray.direction, and
            // cs = cone.cosAngle.  Define C = box.center, U0 = box.axis[0],
            // U1 = box.axis[1], e0 = box.extent[0], and e1 = box.extent[1].
            // A box point is P = C + x*U0 + y*U1 where |x| <= e0 and
            // |y| <= e1.  Define the function
            //   F(x,y) = Dot(D, (P-V)/Length(P-V))
            //   = Dot(D, (x*U0 + y*U1 + (C-V))/|x*U0 + y*U1 + (C-V)|
            //   = (a0*x + a1*y + a2)/(x^2 + y^2 + 2*b0*x + 2*b1*y + b2)^{1/2}
            // The function has an essential singularity when P = V.
            Vector<2, T> diff = box.center - cone.ray.origin;
            T a0 = Dot(cone.ray.direction, box.axis[0]);
            T a1 = Dot(cone.ray.direction, box.axis[1]);
            T a2 = Dot(cone.ray.direction, diff);
            T b0 = Dot(box.axis[0], diff);
            T b1 = Dot(box.axis[1], diff);
            T b2 = Dot(diff, diff);
            T csSqr = cone.cosAngle * cone.cosAngle;

            for (int32_t i1 = 0; i1 < 2; ++i1)
            {
                T sign1 = i1 * (T)2 - (T)1;
                T y = sign1 * box.extent[1];
                for (int32_t i0 = 0; i0 < 2; ++i0)
                {
                    T sign0 = i0 * (T)2 - (T)1;
                    T x = sign0 * box.extent[0];
                    T fNumerator = a0 * x + a1 * y + a2;
                    if (fNumerator > (T)0)
                    {
                        T dSqr = x * x + y * y + (b0 * x + b1 * y) * (T)2 + b2;
                        T nSqr = fNumerator * fNumerator;
                        if (nSqr > dSqr * csSqr)
                        {
                            result.intersect = true;
                            return result;
                        }
                    }
                }
            }

            result.intersect = false;
            return result;
        }
    };
}
