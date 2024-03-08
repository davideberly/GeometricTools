// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a line and a canonical box in 3D.
// 
// The line is P + t * D, where D is not required to be unit length.
// 
// The canonical box has center at the origin and is aligned with the
// coordinate axes. The extents are E = (e[0],e[1],e[2]). A box point is
// Y = (y[0],y[1],y[2]) with |y[i]| <= e[i] for all i.
// 
// The closest point on the line is stored in closest[0] with parameter t. The
// closest point on the box is stored in closest[1]. When there are infinitely
// many choices for the pair of closest points, only one of them is returned.
//
// The DoQueryND functions are described in Section 10.9.4 Linear Component
// to Oriented Bounding Box of
//    Geometric Tools for Computer Graphics,
//    Philip J. Schneider and David H. Eberly,
//    Morgan Kaufmnn, San Francisco CA, 2002
//
// TODO: The code in DistLine2AlignedBox2.h effectively uses the same,
// although in 2D. That code is cleaner than the 3D code here. Clean up the
// 3D code.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/CanonicalBox.h>
#include <Mathematics/Vector3.h>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Line3<T>, CanonicalBox3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                parameter(static_cast<T>(0)),
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {

            }

            T distance, sqrDistance;
            T parameter;
            std::array<Vector3<T>, 2> closest;
        };

        Result operator()(Line3<T> const& line, CanonicalBox3<T> const& box)
        {
            Result result{};

            // Copies are made so that we can transform the line direction to
            // the first octant (nonnegative components) using reflections.
            T const zero = static_cast<T>(0);
            Vector3<T> origin = line.origin;
            Vector3<T> direction = line.direction;
            std::array<bool, 3> reflect{ false, false, false };
            for (int32_t i = 0; i < 3; ++i)
            {
                if (direction[i] < zero)
                {
                    origin[i] = -origin[i];
                    direction[i] = -direction[i];
                    reflect[i] = true;
                }
            }

            // Compute the line-box distance and closest points. The DoQueryND
            // calls compute result.parameter and update result.sqrDistance.
            // The result.distance is computed after the specialized queries.
            // The result.closest[] points are computed afterwards.
            result.sqrDistance = zero;
            result.parameter = zero;
            if (direction[0] > zero)
            {
                if (direction[1] > zero)
                {
                    if (direction[2] > zero)  // (+,+,+)
                    {
                        DoQuery3D(origin, direction, box.extent, result);
                    }
                    else  // (+,+,0)
                    {
                        DoQuery2D(0, 1, 2, origin, direction, box.extent, result);
                    }
                }
                else
                {
                    if (direction[2] > zero)  // (+,0,+)
                    {
                        DoQuery2D(0, 2, 1, origin, direction, box.extent, result);
                    }
                    else  // (+,0,0)
                    {
                        DoQuery1D(0, 1, 2, origin, direction, box.extent, result);
                    }
                }
            }
            else
            {
                if (direction[1] > zero)
                {
                    if (direction[2] > zero)  // (0,+,+)
                    {
                        DoQuery2D(1, 2, 0, origin, direction, box.extent, result);
                    }
                    else  // (0,+,0)
                    {
                        DoQuery1D(1, 0, 2, origin, direction, box.extent, result);
                    }
                }
                else
                {
                    if (direction[2] > zero)  // (0,0,+)
                    {
                        DoQuery1D(2, 0, 1, origin, direction, box.extent, result);
                    }
                    else  // (0,0,0)
                    {
                        DoQuery0D(origin, box.extent, result);
                    }
                }
            }

            // Undo the reflections applied previously.
            for (int32_t i = 0; i < 3; ++i)
            {
                if (reflect[i])
                {
                    origin[i] = -origin[i];
                }
            }

            result.distance = std::sqrt(result.sqrDistance);

            // Compute the closest point on the line.
            result.closest[0] = line.origin + result.parameter * line.direction;

            // Compute the closest point on the box. The original 'origin' is
            // modified by the DoQueryND functions to compute the closest
            // point.
            result.closest[1] = origin;
            return result;
        }

    private:
        static void Face(int32_t i0, int32_t i1, int32_t i2, Vector3<T>& origin,
            Vector3<T> const& direction, Vector3<T> const& PmE,
            Vector3<T> const& extent, Result& result)
        {
            T const zero = static_cast<T>(0);
            T const two = static_cast<T>(2);

            Vector3<T> PpE = origin + extent;

            if (direction[i0] * PpE[i1] >= direction[i1] * PmE[i0])
            {
                if (direction[i0] * PpE[i2] >= direction[i2] * PmE[i0])
                {
                    // v[i1] >= -e[i1], v[i2] >= -e[i2] (distance = 0)
                    origin[i0] = extent[i0];
                    origin[i1] -= direction[i1] * PmE[i0] / direction[i0];
                    origin[i2] -= direction[i2] * PmE[i0] / direction[i0];
                    result.parameter = -PmE[i0] / direction[i0];
                }
                else
                {
                    // v[i1] >= -e[i1], v[i2] < -e[i2]
                    T lenSqr = direction[i0] * direction[i0] +
                        direction[i2] * direction[i2];
                    T tmp = lenSqr * PpE[i1] - direction[i1] * (direction[i0] * PmE[i0] +
                        direction[i2] * PpE[i2]);
                    if (tmp <= two * lenSqr * extent[i1])
                    {
                        T t = tmp / lenSqr;
                        lenSqr += direction[i1] * direction[i1];
                        tmp = PpE[i1] - t;
                        T delta = direction[i0] * PmE[i0] + direction[i1] * tmp +
                            direction[i2] * PpE[i2];
                        result.parameter = -delta / lenSqr;
                        result.sqrDistance += PmE[i0] * PmE[i0] + tmp * tmp +
                            PpE[i2] * PpE[i2] + delta * result.parameter;

                        origin[i0] = extent[i0];
                        origin[i1] = t - extent[i1];
                        origin[i2] = -extent[i2];
                    }
                    else
                    {
                        lenSqr += direction[i1] * direction[i1];
                        T delta = direction[i0] * PmE[i0] + direction[i1] * PmE[i1] +
                            direction[i2] * PpE[i2];
                        result.parameter = -delta / lenSqr;
                        result.sqrDistance += PmE[i0] * PmE[i0] + PmE[i1] * PmE[i1]
                            + PpE[i2] * PpE[i2] + delta * result.parameter;

                        origin[i0] = extent[i0];
                        origin[i1] = extent[i1];
                        origin[i2] = -extent[i2];
                    }
                }
            }
            else
            {
                if (direction[i0] * PpE[i2] >= direction[i2] * PmE[i0])
                {
                    // v[i1] < -e[i1], v[i2] >= -e[i2]
                    T lenSqr = direction[i0] * direction[i0] +
                        direction[i1] * direction[i1];
                    T tmp = lenSqr * PpE[i2] - direction[i2] * (direction[i0] * PmE[i0] +
                        direction[i1] * PpE[i1]);
                    if (tmp <= two * lenSqr * extent[i2])
                    {
                        T t = tmp / lenSqr;
                        lenSqr += direction[i2] * direction[i2];
                        tmp = PpE[i2] - t;
                        T delta = direction[i0] * PmE[i0] + direction[i1] * PpE[i1] +
                            direction[i2] * tmp;
                        result.parameter = -delta / lenSqr;
                        result.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1] +
                            tmp * tmp + delta * result.parameter;

                        origin[i0] = extent[i0];
                        origin[i1] = -extent[i1];
                        origin[i2] = t - extent[i2];
                    }
                    else
                    {
                        lenSqr += direction[i2] * direction[i2];
                        T delta = direction[i0] * PmE[i0] + direction[i1] * PpE[i1] +
                            direction[i2] * PmE[i2];
                        result.parameter = -delta / lenSqr;
                        result.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1] +
                            PmE[i2] * PmE[i2] + delta * result.parameter;

                        origin[i0] = extent[i0];
                        origin[i1] = -extent[i1];
                        origin[i2] = extent[i2];
                    }
                }
                else
                {
                    // v[i1] < -e[i1], v[i2] < -e[i2]
                    T lenSqr = direction[i0] * direction[i0] +
                        direction[i2] * direction[i2];
                    T tmp = lenSqr * PpE[i1] - direction[i1] * (direction[i0] * PmE[i0] +
                        direction[i2] * PpE[i2]);
                    if (tmp >= zero)
                    {
                        // v[i1]-edge is closest
                        if (tmp <= two * lenSqr * extent[i1])
                        {
                            T t = tmp / lenSqr;
                            lenSqr += direction[i1] * direction[i1];
                            tmp = PpE[i1] - t;
                            T delta = direction[i0] * PmE[i0] + direction[i1] * tmp +
                                direction[i2] * PpE[i2];
                            result.parameter = -delta / lenSqr;
                            result.sqrDistance += PmE[i0] * PmE[i0] + tmp * tmp +
                                PpE[i2] * PpE[i2] + delta * result.parameter;

                            origin[i0] = extent[i0];
                            origin[i1] = t - extent[i1];
                            origin[i2] = -extent[i2];
                        }
                        else
                        {
                            lenSqr += direction[i1] * direction[i1];
                            T delta = direction[i0] * PmE[i0] + direction[i1] * PmE[i1]
                                + direction[i2] * PpE[i2];
                            result.parameter = -delta / lenSqr;
                            result.sqrDistance += PmE[i0] * PmE[i0] + PmE[i1] * PmE[i1]
                                + PpE[i2] * PpE[i2] + delta * result.parameter;

                            origin[i0] = extent[i0];
                            origin[i1] = extent[i1];
                            origin[i2] = -extent[i2];
                        }
                        return;
                    }

                    lenSqr = direction[i0] * direction[i0] +
                        direction[i1] * direction[i1];
                    tmp = lenSqr * PpE[i2] - direction[i2] * (direction[i0] * PmE[i0] +
                        direction[i1] * PpE[i1]);
                    if (tmp >= zero)
                    {
                        // v[i2]-edge is closest
                        if (tmp <= two * lenSqr * extent[i2])
                        {
                            T t = tmp / lenSqr;
                            lenSqr += direction[i2] * direction[i2];
                            tmp = PpE[i2] - t;
                            T delta = direction[i0] * PmE[i0] + direction[i1] * PpE[i1] +
                                direction[i2] * tmp;
                            result.parameter = -delta / lenSqr;
                            result.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1] +
                                tmp * tmp + delta * result.parameter;

                            origin[i0] = extent[i0];
                            origin[i1] = -extent[i1];
                            origin[i2] = t - extent[i2];
                        }
                        else
                        {
                            lenSqr += direction[i2] * direction[i2];
                            T delta = direction[i0] * PmE[i0] + direction[i1] * PpE[i1]
                                + direction[i2] * PmE[i2];
                            result.parameter = -delta / lenSqr;
                            result.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1]
                                + PmE[i2] * PmE[i2] + delta * result.parameter;

                            origin[i0] = extent[i0];
                            origin[i1] = -extent[i1];
                            origin[i2] = extent[i2];
                        }
                        return;
                    }

                    // (v[i1],v[i2])-corner is closest
                    lenSqr += direction[i2] * direction[i2];
                    T delta = direction[i0] * PmE[i0] + direction[i1] * PpE[i1] +
                        direction[i2] * PpE[i2];
                    result.parameter = -delta / lenSqr;
                    result.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1]
                        + PpE[i2] * PpE[i2] + delta * result.parameter;

                    origin[i0] = extent[i0];
                    origin[i1] = -extent[i1];
                    origin[i2] = -extent[i2];
                }
            }
        }

        static void DoQuery3D(Vector3<T>& origin, Vector3<T> const& direction,
            Vector3<T> const& extent, Result& result)
        {
            Vector3<T> PmE = origin - extent;
            T prodDxPy = direction[0] * PmE[1];
            T prodDyPx = direction[1] * PmE[0];

            if (prodDyPx >= prodDxPy)
            {
                T prodDzPx = direction[2] * PmE[0];
                T prodDxPz = direction[0] * PmE[2];
                if (prodDzPx >= prodDxPz)
                {
                    // line intersects x = e0
                    Face(0, 1, 2, origin, direction, PmE, extent, result);
                }
                else
                {
                    // line intersects z = e2
                    Face(2, 0, 1, origin, direction, PmE, extent, result);
                }
            }
            else
            {
                T prodDzPy = direction[2] * PmE[1];
                T prodDyPz = direction[1] * PmE[2];
                if (prodDzPy >= prodDyPz)
                {
                    // line intersects y = e1
                    Face(1, 2, 0, origin, direction, PmE, extent, result);
                }
                else
                {
                    // line intersects z = e2
                    Face(2, 0, 1, origin, direction, PmE, extent, result);
                }
            }
        }

        static void DoQuery2D(int32_t i0, int32_t i1, int32_t i2, Vector3<T>& origin,
            Vector3<T> const& direction, Vector3<T> const& extent, Result& result)
        {
            T const zero = static_cast<T>(0);

            T PmE0 = origin[i0] - extent[i0];
            T PmE1 = origin[i1] - extent[i1];
            T prod0 = direction[i1] * PmE0;
            T prod1 = direction[i0] * PmE1;

            if (prod0 >= prod1)
            {
                // line intersects P[i0] = e[i0]
                origin[i0] = extent[i0];

                T PpE1 = origin[i1] + extent[i1];
                T delta = prod0 - direction[i0] * PpE1;
                if (delta >= zero)
                {
                    T lenSqr = direction[i0] * direction[i0] +
                        direction[i1] * direction[i1];
                    result.sqrDistance += delta * delta / lenSqr;
                    origin[i1] = -extent[i1];
                    result.parameter = -(direction[i0] * PmE0 +
                        direction[i1] * PpE1) / lenSqr;
                }
                else
                {
                    origin[i1] -= prod0 / direction[i0];
                    result.parameter = -PmE0 / direction[i0];
                }
            }
            else
            {
                // line intersects P[i1] = e[i1]
                origin[i1] = extent[i1];

                T PpE0 = origin[i0] + extent[i0];
                T delta = prod1 - direction[i1] * PpE0;
                if (delta >= zero)
                {
                    T lenSqr = direction[i0] * direction[i0] +
                        direction[i1] * direction[i1];
                    result.sqrDistance += delta * delta / lenSqr;
                    origin[i0] = -extent[i0];
                    result.parameter = -(direction[i0] * PpE0 +
                        direction[i1] * PmE1) / lenSqr;
                }
                else
                {
                    origin[i0] -= prod1 / direction[i1];
                    result.parameter = -PmE1 / direction[i1];
                }
            }

            if (origin[i2] < -extent[i2])
            {
                T delta = origin[i2] + extent[i2];
                result.sqrDistance += delta * delta;
                origin[i2] = -extent[i2];
            }
            else if (origin[i2] > extent[i2])
            {
                T delta = origin[i2] - extent[i2];
                result.sqrDistance += delta * delta;
                origin[i2] = extent[i2];
            }
        }

        static void DoQuery1D(int32_t i0, int32_t i1, int32_t i2, Vector3<T>& origin,
            Vector3<T> const& direction, Vector3<T> const& extent, Result& result)
        {
            result.parameter = (extent[i0] - origin[i0]) / direction[i0];

            origin[i0] = extent[i0];
            for (auto i : { i1, i2 })
            {
                if (origin[i] < -extent[i])
                {
                    T delta = origin[i] + extent[i];
                    result.sqrDistance += delta * delta;
                    origin[i] = -extent[i];
                }
                else if (origin[i] > extent[i])
                {
                    T delta = origin[i] - extent[i];
                    result.sqrDistance += delta * delta;
                    origin[i] = extent[i];
                }
            }
        }

        static void DoQuery0D(Vector3<T>& origin, Vector3<T> const& extent, Result& result)
        {
            for (int32_t i = 0; i < 3; ++i)
            {
                if (origin[i] < -extent[i])
                {
                    T delta = origin[i] + extent[i];
                    result.sqrDistance += delta * delta;
                    origin[i] = -extent[i];
                }
                else if (origin[i] > extent[i])
                {
                    T delta = origin[i] - extent[i];
                    result.sqrDistance += delta * delta;
                    origin[i] = extent[i];
                }
            }
        }
    };
}
