// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a plane and a solid canonical box in 3D.
// 
// The plane is defined by Dot(N, X - P) = 0, where P is the plane origin and
// N is a unit-length normal for the plane.
// 
// The canonical box has center at the origin and is aligned with the
// coordinate axes. The extents are E = (e[0],e[1],e[2]). A box point is
// Y = (y[0],y[1],y[2]) with |y[i]| <= e[i] for all i.
// 
// The closest point on the plane is stored in closest[0]. The closest point
// on the box is stored in closest[1]. When there are infinitely many choices
// for the pair of closest points, only one of them is returned.
//
// TODO: Modify to support non-unit-length N.

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/3D/Plane3.h>
#include <GTL/Mathematics/Primitives/ND/CanonicalBox.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Plane3<T>, CanonicalBox3<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<Vector3<T>, 2> closest;
        };

        Output operator()(Plane3<T> const& plane, CanonicalBox3<T> const& box)
        {
            Output output{};

            // Copies are made so that we can transform the plane normal to
            // the first octant (nonnegative components) using reflections.
            Vector3<T> origin = plane.constant * plane.normal;
            Vector3<T> normal = plane.normal;
            std::array<bool, 3> reflect{ false, false };
            for (size_t i = 0; i < 3; ++i)
            {
                if (normal[i] < C_<T>(0))
                {
                    origin[i] = -origin[i];
                    normal[i] = -normal[i];
                    reflect[i] = true;
                }
            }

            // Compute the plane-box closest points.
            if (normal[0] > C_<T>(0))
            {
                if (normal[1] > C_<T>(0))
                {
                    if (normal[2] > C_<T>(0))
                    {
                        // The normal signs are (+,+,+).
                        DoQuery3D(origin, normal, box.extent, output);
                    }
                    else
                    {
                        // The normal signs are (+,+,0).
                        DoQuery2D(0, 1, 2, origin, normal, box.extent, output);
                    }
                }
                else
                {
                    if (normal[2] > C_<T>(0))
                    {
                        // The normal signs are (+,0,+).
                        DoQuery2D(0, 2, 1, origin, normal, box.extent, output);
                    }
                    else
                    {
                        // The normal signs are (+,0,0). The closest box point
                        // is (x0,e1,e2) where x0 = clamp(p0,[-e0,e0]). The
                        // closest plane point is (p0,e1,e2).
                        DoQuery1D(0, 1, 2, origin, box.extent, output);
                    }
                }
            }
            else
            {
                if (normal[1] > C_<T>(0))
                {
                    if (normal[2] > C_<T>(0))
                    {
                        // The normal signs are (0,+,+).
                        DoQuery2D(1, 2, 0, origin, normal, box.extent, output);
                    }
                    else
                    {
                        // The normal signs are (0,+,0). The closest box point
                        // is (e0,x1,e2) where x1 = clamp(p1,[-e1,e1]). The
                        // closest plane point is (e0,p1,e2).
                        DoQuery1D(1, 2, 0, origin, box.extent, output);
                    }
                }
                else
                {
                    if (normal[2] > C_<T>(0))
                    {
                        // The normal signs are (0,0,+) The closest box point
                        // is (e0,e1,x2) where x2 = clamp(p2,[-e2,e2]). The
                        // closest plane point is (e0,e1,p2).
                        DoQuery1D(2, 0, 1, origin, box.extent, output);
                    }
                    else
                    {
                        // The normal signs are (0,0,0). Execute the DCP
                        // query for the plane origin and the canonical box.
                        // This is a low-probability event.
                        DoQuery0D(plane.origin, box.extent, output);
                    }
                }
            }

            // Undo the reflections. The origin and normal are not
            // consumed, so these do not need to be reflected. However, the
            // closest points are consumed.
            for (size_t i = 0; i < 3; ++i)
            {
                if (reflect[i])
                {
                    for (size_t j = 0; j < 2; ++j)
                    {
                        output.closest[j][i] = -output.closest[j][i];
                    }
                }
            }

            Vector3<T> diff = output.closest[0] - output.closest[1];
            output.sqrDistance = Dot(diff, diff);
            output.distance = std::sqrt(output.sqrDistance);
            return output;
        }

    private:
        static void DoQuery3D(Vector3<T> const& origin, Vector3<T> const& normal,
            Vector3<T> const& extent, Output& output)
        {
            T dmin = -Dot(normal, extent + origin);
            if (dmin >= C_<T>(0))
            {
                output.closest[0] = -extent - dmin * normal;
                output.closest[1] = -extent;
            }
            else
            {
                T dmax = Dot(normal, extent - origin);
                if (dmax <= C_<T>(0))
                {
                    output.closest[0] = extent - dmax * normal;
                    output.closest[1] = extent;
                }
                else
                {
                    // t = dmin / (dmin - dmax) in [0,1], compute s = 2*t-1
                    T s = C_<T>(2) * dmin / (dmin - dmax) - C_<T>(1);
                    output.closest[0] = s * extent;
                    output.closest[1] = output.closest[0];
                }
            }
        }

        static void DoQuery2D(int32_t i0, int32_t i1, int32_t i2,
            Vector3<T> const& origin, Vector3<T> const& normal,
            Vector3<T> const& extent, Output& output)
        {
            T dmin = -(
                normal[i0] * (extent[i0] + origin[i0]) +
                normal[i1] * (extent[i1] + origin[i1]));

            if (dmin >= C_<T>(0))
            {
                output.closest[0][i0] = -extent[i0] - dmin * normal[i0];
                output.closest[0][i1] = -extent[i1] - dmin * normal[i1];
                output.closest[0][i2] = extent[i2];
                output.closest[1][i0] = -extent[i0];
                output.closest[1][i1] = -extent[i1];
                output.closest[1][i2] = extent[i2];
            }
            else
            {
                T dmax = (
                    normal[i0] * (extent[i0] - origin[i0]) +
                    normal[i1] * (extent[i1] - origin[i1]));

                if (dmax <= C_<T>(0))
                {
                    output.closest[0][i0] = extent[i0] - dmax * normal[i0];
                    output.closest[0][i1] = extent[i1] - dmax * normal[i1];
                    output.closest[0][i2] = extent[i2];
                    output.closest[1][i0] = extent[i0];
                    output.closest[1][i1] = extent[i1];
                    output.closest[1][i2] = extent[i2];
                }
                else
                {
                    // t = dmin / (dmin - dmax) in [0,1], compute s = 2*t-1
                    T s = C_<T>(2) * dmin / (dmin - dmax) - C_<T>(1);
                    output.closest[0][i0] = s * extent[i0];
                    output.closest[0][i1] = s * extent[i1];
                    output.closest[0][i2] = extent[i2];
                    output.closest[1] = output.closest[0];
                }
            }
        }

        static void DoQuery1D(int32_t i0, int32_t i1, int32_t i2,
            Vector3<T> const& origin, Vector3<T> const& extent, Output& output)
        {
            output.closest[0][i0] = origin[i0];
            output.closest[0][i1] = extent[i1];
            output.closest[0][i2] = extent[i2];
            output.closest[1][i0] = std::max(-extent[i0], std::min(origin[i0], extent[i0]));
            output.closest[1][i1] = extent[i1];
            output.closest[1][i2] = extent[i2];
        }

        static void DoQuery0D(Vector3<T> const& origin, Vector3<T> const& extent,
            Output& output)
        {
            output.closest[0] = origin;
            output.closest[1][0] = std::max(-extent[0], std::min(origin[0], extent[0]));
            output.closest[1][1] = std::max(-extent[1], std::min(origin[1], extent[1]));
            output.closest[1][2] = std::max(-extent[2], std::min(origin[2], extent[2]));
        }
    };
}
