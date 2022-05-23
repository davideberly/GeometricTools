// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

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

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/CanonicalBox.h>
#include <GTL/Mathematics/Primitives/ND/Line.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Line3<T>, CanonicalBox3<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                parameter(C_<T>(0)),
                closest{}
            {

            }

            T distance, sqrDistance;
            T parameter;
            std::array<Vector3<T>, 2> closest;
        };

        Output operator()(Line3<T> const& line, CanonicalBox3<T> const& box)
        {
            Output output{};

            // Copies are made so that we can transform the line direction to
            // the first octant (nonnegative components) using reflections.
            Vector3<T> origin = line.origin;
            Vector3<T> direction = line.direction;
            std::array<bool, 3> reflect{ false, false, false };
            for (size_t i = 0; i < 3; ++i)
            {
                if (direction[i] < C_<T>(0))
                {
                    origin[i] = -origin[i];
                    direction[i] = -direction[i];
                    reflect[i] = true;
                }
            }

            // Compute the line-box distance and closest points. The DoQueryND
            // calls compute output.parameter and update output.sqrDistance.
            // The output.distance is computed after the specialized queries.
            // The output.closest[] points are computed afterwards.
            output.sqrDistance = C_<T>(0);
            output.parameter = C_<T>(0);
            if (direction[0] > C_<T>(0))
            {
                if (direction[1] > C_<T>(0))
                {
                    if (direction[2] > C_<T>(0))  // (+,+,+)
                    {
                        DoQuery3D(origin, direction, box.extent, output);
                    }
                    else  // (+,+,0)
                    {
                        DoQuery2D(0, 1, 2, origin, direction, box.extent, output);
                    }
                }
                else
                {
                    if (direction[2] > C_<T>(0))  // (+,0,+)
                    {
                        DoQuery2D(0, 2, 1, origin, direction, box.extent, output);
                    }
                    else  // (+,0,0)
                    {
                        DoQuery1D(0, 1, 2, origin, direction, box.extent, output);
                    }
                }
            }
            else
            {
                if (direction[1] > C_<T>(0))
                {
                    if (direction[2] > C_<T>(0))  // (0,+,+)
                    {
                        DoQuery2D(1, 2, 0, origin, direction, box.extent, output);
                    }
                    else  // (0,+,0)
                    {
                        DoQuery1D(1, 0, 2, origin, direction, box.extent, output);
                    }
                }
                else
                {
                    if (direction[2] > C_<T>(0))  // (0,0,+)
                    {
                        DoQuery1D(2, 0, 1, origin, direction, box.extent, output);
                    }
                    else  // (0,0,0)
                    {
                        DoQuery0D(origin, box.extent, output);
                    }
                }
            }

            // Undo the reflections applied previously.
            for (size_t i = 0; i < 3; ++i)
            {
                if (reflect[i])
                {
                    origin[i] = -origin[i];
                }
            }

            output.distance = std::sqrt(output.sqrDistance);

            // Compute the closest point on the line.
            output.closest[0] = line.origin + output.parameter * line.direction;

            // Compute the closest point on the box. The original 'origin' is
            // modified by the DoQueryND functions to compute the closest
            // point.
            output.closest[1] = origin;
            return output;
        }

    private:
        static void Face(size_t i0, size_t i1, size_t i2, Vector3<T>& origin,
            Vector3<T> const& direction, Vector3<T> const& PmE,
            Vector3<T> const& extent, Output& output)
        {
            Vector3<T> PpE = origin + extent;

            if (direction[i0] * PpE[i1] >= direction[i1] * PmE[i0])
            {
                if (direction[i0] * PpE[i2] >= direction[i2] * PmE[i0])
                {
                    // v[i1] >= -e[i1], v[i2] >= -e[i2] (distance = 0)
                    origin[i0] = extent[i0];
                    origin[i1] -= direction[i1] * PmE[i0] / direction[i0];
                    origin[i2] -= direction[i2] * PmE[i0] / direction[i0];
                    output.parameter = -PmE[i0] / direction[i0];
                }
                else
                {
                    // v[i1] >= -e[i1], v[i2] < -e[i2]
                    T lenSqr = direction[i0] * direction[i0] +
                        direction[i2] * direction[i2];
                    T tmp = lenSqr * PpE[i1] - direction[i1] * (direction[i0] * PmE[i0] +
                        direction[i2] * PpE[i2]);
                    if (tmp <= C_<T>(2) * lenSqr * extent[i1])
                    {
                        T t = tmp / lenSqr;
                        lenSqr += direction[i1] * direction[i1];
                        tmp = PpE[i1] - t;
                        T delta = direction[i0] * PmE[i0] + direction[i1] * tmp +
                            direction[i2] * PpE[i2];
                        output.parameter = -delta / lenSqr;
                        output.sqrDistance += PmE[i0] * PmE[i0] + tmp * tmp +
                            PpE[i2] * PpE[i2] + delta * output.parameter;

                        origin[i0] = extent[i0];
                        origin[i1] = t - extent[i1];
                        origin[i2] = -extent[i2];
                    }
                    else
                    {
                        lenSqr += direction[i1] * direction[i1];
                        T delta = direction[i0] * PmE[i0] + direction[i1] * PmE[i1] +
                            direction[i2] * PpE[i2];
                        output.parameter = -delta / lenSqr;
                        output.sqrDistance += PmE[i0] * PmE[i0] + PmE[i1] * PmE[i1]
                            + PpE[i2] * PpE[i2] + delta * output.parameter;

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
                    if (tmp <= C_<T>(2) * lenSqr * extent[i2])
                    {
                        T t = tmp / lenSqr;
                        lenSqr += direction[i2] * direction[i2];
                        tmp = PpE[i2] - t;
                        T delta = direction[i0] * PmE[i0] + direction[i1] * PpE[i1] +
                            direction[i2] * tmp;
                        output.parameter = -delta / lenSqr;
                        output.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1] +
                            tmp * tmp + delta * output.parameter;

                        origin[i0] = extent[i0];
                        origin[i1] = -extent[i1];
                        origin[i2] = t - extent[i2];
                    }
                    else
                    {
                        lenSqr += direction[i2] * direction[i2];
                        T delta = direction[i0] * PmE[i0] + direction[i1] * PpE[i1] +
                            direction[i2] * PmE[i2];
                        output.parameter = -delta / lenSqr;
                        output.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1] +
                            PmE[i2] * PmE[i2] + delta * output.parameter;

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
                    if (tmp >= C_<T>(0))
                    {
                        // v[i1]-edge is closest
                        if (tmp <= C_<T>(2) * lenSqr * extent[i1])
                        {
                            T t = tmp / lenSqr;
                            lenSqr += direction[i1] * direction[i1];
                            tmp = PpE[i1] - t;
                            T delta = direction[i0] * PmE[i0] + direction[i1] * tmp +
                                direction[i2] * PpE[i2];
                            output.parameter = -delta / lenSqr;
                            output.sqrDistance += PmE[i0] * PmE[i0] + tmp * tmp +
                                PpE[i2] * PpE[i2] + delta * output.parameter;

                            origin[i0] = extent[i0];
                            origin[i1] = t - extent[i1];
                            origin[i2] = -extent[i2];
                        }
                        else
                        {
                            lenSqr += direction[i1] * direction[i1];
                            T delta = direction[i0] * PmE[i0] + direction[i1] * PmE[i1]
                                + direction[i2] * PpE[i2];
                            output.parameter = -delta / lenSqr;
                            output.sqrDistance += PmE[i0] * PmE[i0] + PmE[i1] * PmE[i1]
                                + PpE[i2] * PpE[i2] + delta * output.parameter;

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
                    if (tmp >= C_<T>(0))
                    {
                        // v[i2]-edge is closest
                        if (tmp <= C_<T>(2) * lenSqr * extent[i2])
                        {
                            T t = tmp / lenSqr;
                            lenSqr += direction[i2] * direction[i2];
                            tmp = PpE[i2] - t;
                            T delta = direction[i0] * PmE[i0] + direction[i1] * PpE[i1] +
                                direction[i2] * tmp;
                            output.parameter = -delta / lenSqr;
                            output.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1] +
                                tmp * tmp + delta * output.parameter;

                            origin[i0] = extent[i0];
                            origin[i1] = -extent[i1];
                            origin[i2] = t - extent[i2];
                        }
                        else
                        {
                            lenSqr += direction[i2] * direction[i2];
                            T delta = direction[i0] * PmE[i0] + direction[i1] * PpE[i1]
                                + direction[i2] * PmE[i2];
                            output.parameter = -delta / lenSqr;
                            output.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1]
                                + PmE[i2] * PmE[i2] + delta * output.parameter;

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
                    output.parameter = -delta / lenSqr;
                    output.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1]
                        + PpE[i2] * PpE[i2] + delta * output.parameter;

                    origin[i0] = extent[i0];
                    origin[i1] = -extent[i1];
                    origin[i2] = -extent[i2];
                }
            }
        }

        static void DoQuery3D(Vector3<T>& origin, Vector3<T> const& direction,
            Vector3<T> const& extent, Output& output)
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
                    Face(0, 1, 2, origin, direction, PmE, extent, output);
                }
                else
                {
                    // line intersects z = e2
                    Face(2, 0, 1, origin, direction, PmE, extent, output);
                }
            }
            else
            {
                T prodDzPy = direction[2] * PmE[1];
                T prodDyPz = direction[1] * PmE[2];
                if (prodDzPy >= prodDyPz)
                {
                    // line intersects y = e1
                    Face(1, 2, 0, origin, direction, PmE, extent, output);
                }
                else
                {
                    // line intersects z = e2
                    Face(2, 0, 1, origin, direction, PmE, extent, output);
                }
            }
        }

        static void DoQuery2D(size_t i0, size_t i1, size_t i2, Vector3<T>& origin,
            Vector3<T> const& direction, Vector3<T> const& extent, Output& output)
        {
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
                if (delta >= C_<T>(0))
                {
                    T lenSqr = direction[i0] * direction[i0] +
                        direction[i1] * direction[i1];
                    output.sqrDistance += delta * delta / lenSqr;
                    origin[i1] = -extent[i1];
                    output.parameter = -(direction[i0] * PmE0 +
                        direction[i1] * PpE1) / lenSqr;
                }
                else
                {
                    origin[i1] -= prod0 / direction[i0];
                    output.parameter = -PmE0 / direction[i0];
                }
            }
            else
            {
                // line intersects P[i1] = e[i1]
                origin[i1] = extent[i1];

                T PpE0 = origin[i0] + extent[i0];
                T delta = prod1 - direction[i1] * PpE0;
                if (delta >= C_<T>(0))
                {
                    T lenSqr = direction[i0] * direction[i0] +
                        direction[i1] * direction[i1];
                    output.sqrDistance += delta * delta / lenSqr;
                    origin[i0] = -extent[i0];
                    output.parameter = -(direction[i0] * PpE0 +
                        direction[i1] * PmE1) / lenSqr;
                }
                else
                {
                    origin[i0] -= prod1 / direction[i1];
                    output.parameter = -PmE1 / direction[i1];
                }
            }

            if (origin[i2] < -extent[i2])
            {
                T delta = origin[i2] + extent[i2];
                output.sqrDistance += delta * delta;
                origin[i2] = -extent[i2];
            }
            else if (origin[i2] > extent[i2])
            {
                T delta = origin[i2] - extent[i2];
                output.sqrDistance += delta * delta;
                origin[i2] = extent[i2];
            }
        }

        static void DoQuery1D(size_t i0, size_t i1, size_t i2, Vector3<T>& origin,
            Vector3<T> const& direction, Vector3<T> const& extent, Output& output)
        {
            output.parameter = (extent[i0] - origin[i0]) / direction[i0];

            origin[i0] = extent[i0];
            for (auto i : { i1, i2 })
            {
                if (origin[i] < -extent[i])
                {
                    T delta = origin[i] + extent[i];
                    output.sqrDistance += delta * delta;
                    origin[i] = -extent[i];
                }
                else if (origin[i] > extent[i])
                {
                    T delta = origin[i] - extent[i];
                    output.sqrDistance += delta * delta;
                    origin[i] = extent[i];
                }
            }
        }

        static void DoQuery0D(Vector3<T>& origin, Vector3<T> const& extent, Output& output)
        {
            for (size_t i = 0; i < 3; ++i)
            {
                if (origin[i] < -extent[i])
                {
                    T delta = origin[i] + extent[i];
                    output.sqrDistance += delta * delta;
                    origin[i] = -extent[i];
                }
                else if (origin[i] > extent[i])
                {
                    T delta = origin[i] - extent[i];
                    output.sqrDistance += delta * delta;
                    origin[i] = extent[i];
                }
            }
        }
    };
}
