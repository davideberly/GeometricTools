// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a point and a solid triangle in nD.
// 
// The triangle has vertices <V[0],V[1],V[2]>. A triangle point is
// X = sum_{i=0}^2 b[i] * V[i], where 0 <= b[i] <= 1 for all i and
// sum_{i=0}^2 b[i] = 1.
// 
// The input point is stored in closest[0]. The closest point on the triangle
// is stored in closest[1] with barycentric coordinates (b[0],b[1],b[2]).
// 
// For a description of the algebraic details of the quadratic minimization
// approach used by operator(), see
//   https://www.geometrictools.com/Documentation/DistancePoint3Triangle3.pdf
// Although the document describes the 3D case, the construction applies in
// general dimensions N. The UseConjugateGradient function uses conjugate
// gradient minimization.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Triangle.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Vector<N, T>, Triangle<N, T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                barycentric{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                closest{ Vector<N, T>::Zero(), Vector<N, T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<T, 3> barycentric;
            std::array<Vector<N, T>, 2> closest;
        };

        // This query is exact when using arbitrary-precision arithmetic. It
        // can be used also for floating-point arithmetic, but rounding errors
        // can sometimes lead to inaccurate result. For floating-point,
        // consider UseConjugateGradient(...) which is more robust.
        Result operator()(Vector<N, T> const& point, Triangle<N, T> const& triangle)
        {
            // The member result.sqrDistance is set in each block of the
            // nested if-then-else statements. The remaining members are all
            // set at the end of the function.
            Result result{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const two = static_cast<T>(2);
            Vector<N, T> diff = triangle.v[0] - point;
            Vector<N, T> edge0 = triangle.v[1] - triangle.v[0];
            Vector<N, T> edge1 = triangle.v[2] - triangle.v[0];
            T a00 = Dot(edge0, edge0);
            T a01 = Dot(edge0, edge1);
            T a11 = Dot(edge1, edge1);
            T b0 = Dot(diff, edge0);
            T b1 = Dot(diff, edge1);
            T det = std::max(a00 * a11 - a01 * a01, zero);
            T s = a01 * b1 - a11 * b0;
            T t = a01 * b0 - a00 * b1;

            if (s + t <= det)
            {
                if (s < zero)
                {
                    if (t < zero)  // region 4
                    {
                        if (b0 < zero)
                        {
                            t = zero;
                            if (-b0 >= a00)
                            {
                                s = one;
                            }
                            else
                            {
                                s = -b0 / a00;
                            }
                        }
                        else
                        {
                            s = zero;
                            if (b1 >= zero)
                            {
                                t = zero;
                            }
                            else if (-b1 >= a11)
                            {
                                t = one;
                            }
                            else
                            {
                                t = -b1 / a11;
                            }
                        }
                    }
                    else  // region 3
                    {
                        s = zero;
                        if (b1 >= zero)
                        {
                            t = zero;
                        }
                        else if (-b1 >= a11)
                        {
                            t = one;
                        }
                        else
                        {
                            t = -b1 / a11;
                        }
                    }
                }
                else if (t < zero)  // region 5
                {
                    t = zero;
                    if (b0 >= zero)
                    {
                        s = zero;
                    }
                    else if (-b0 >= a00)
                    {
                        s = one;
                    }
                    else
                    {
                        s = -b0 / a00;
                    }
                }
                else  // region 0
                {
                    // minimum at interior point
                    s /= det;
                    t /= det;
                }
            }
            else
            {
                T tmp0{}, tmp1{}, numer{}, denom{};

                if (s < zero)  // region 2
                {
                    tmp0 = a01 + b0;
                    tmp1 = a11 + b1;
                    if (tmp1 > tmp0)
                    {
                        numer = tmp1 - tmp0;
                        denom = a00 - two * a01 + a11;
                        if (numer >= denom)
                        {
                            s = one;
                            t = zero;
                        }
                        else
                        {
                            s = numer / denom;
                            t = one - s;
                        }
                    }
                    else
                    {
                        s = zero;
                        if (tmp1 <= zero)
                        {
                            t = one;
                        }
                        else if (b1 >= zero)
                        {
                            t = zero;
                        }
                        else
                        {
                            t = -b1 / a11;
                        }
                    }
                }
                else if (t < zero)  // region 6
                {
                    tmp0 = a01 + b1;
                    tmp1 = a00 + b0;
                    if (tmp1 > tmp0)
                    {
                        numer = tmp1 - tmp0;
                        denom = a00 - two * a01 + a11;
                        if (numer >= denom)
                        {
                            t = one;
                            s = zero;
                        }
                        else
                        {
                            t = numer / denom;
                            s = one - t;
                        }
                    }
                    else
                    {
                        t = zero;
                        if (tmp1 <= zero)
                        {
                            s = one;
                        }
                        else if (b0 >= zero)
                        {
                            s = zero;
                        }
                        else
                        {
                            s = -b0 / a00;
                        }
                    }
                }
                else  // region 1
                {
                    numer = a11 + b1 - a01 - b0;
                    if (numer <= zero)
                    {
                        s = zero;
                        t = one;
                    }
                    else
                    {
                        denom = a00 - two * a01 + a11;
                        if (numer >= denom)
                        {
                            s = one;
                            t = zero;
                        }
                        else
                        {
                            s = numer / denom;
                            t = one - s;
                        }
                    }
                }
            }

            result.closest[0] = point;
            result.closest[1] = triangle.v[0] + s * edge0 + t * edge1;
            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            result.barycentric[0] = one - s - t;
            result.barycentric[1] = s;
            result.barycentric[2] = t;
            return result;
        }

        // The query is designed to be robust when using floating-point
        // arithmetic. For arbitrary-precision arithmetic, use the function
        // operator()(...).
        Result UseConjugateGradient(Vector<N, T> const& point,
            Triangle<N, T> const& triangle)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            Vector<N, T> diff = point - triangle.v[0];
            Vector<N, T> edge0 = triangle.v[1] - triangle.v[0];
            Vector<N, T> edge1 = triangle.v[2] - triangle.v[0];
            T a00 = Dot(edge0, edge0);
            T a01 = Dot(edge0, edge1);
            T a11 = Dot(edge1, edge1);
            T b0 = -Dot(diff, edge0);
            T b1 = -Dot(diff, edge1);

            T f00 = b0;
            T f10 = b0 + a00;
            T f01 = b0 + a01;

            std::array<T, 2> p0{}, p1{}, p{};
            T dt1{}, h0{}, h1{};

            // Compute the endpoints p0 and p1 of the segment. The segment is
            // parameterized by L(z) = (1-z)*p0 + z*p1 for z in [0,1] and the
            // directional derivative of half the quadratic on the segment is
            // H(z) = Dot(p1-p0,gradient[Q](L(z))/2), where gradient[Q]/2 =
            // (F,G). By design, F(L(z)) = 0 for cases (2), (4), (5), and
            // (6). Cases (1) and (3) can correspond to no-intersection or
            // intersection of F = 0 with the triangle.
            if (f00 >= zero)
            {
                if (f01 >= zero)
                {
                    // (1) p0 = (0,0), p1 = (0,1), H(z) = G(L(z))
                    GetMinEdge02(a11, b1, p);
                }
                else
                {
                    // (2) p0 = (0,t10), p1 = (t01,1-t01),
                    // H(z) = (t11 - t10)*G(L(z))
                    p0[0] = zero;
                    p0[1] = f00 / (f00 - f01);
                    p1[0] = f01 / (f01 - f10);
                    p1[1] = one - p1[0];
                    dt1 = p1[1] - p0[1];
                    h0 = dt1 * (a11 * p0[1] + b1);
                    if (h0 >= zero)
                    {
                        GetMinEdge02(a11, b1, p);
                    }
                    else
                    {
                        h1 = dt1 * (a01 * p1[0] + a11 * p1[1] + b1);
                        if (h1 <= zero)
                        {
                            GetMinEdge12(a01, a11, b1, f10, f01, p);
                        }
                        else
                        {
                            GetMinInterior(p0, h0, p1, h1, p);
                        }
                    }
                }
            }
            else if (f01 <= zero)
            {
                if (f10 <= zero)
                {
                    // (3) p0 = (1,0), p1 = (0,1), H(z) = G(L(z)) - F(L(z))
                    GetMinEdge12(a01, a11, b1, f10, f01, p);
                }
                else
                {
                    // (4) p0 = (t00,0), p1 = (t01,1-t01), H(z) = t11*G(L(z))
                    p0[0] = f00 / (f00 - f10);
                    p0[1] = zero;
                    p1[0] = f01 / (f01 - f10);
                    p1[1] = one - p1[0];
                    h0 = p1[1] * (a01 * p0[0] + b1);
                    if (h0 >= zero)
                    {
                        p = p0;  // GetMinEdge01
                    }
                    else
                    {
                        h1 = p1[1] * (a01 * p1[0] + a11 * p1[1] + b1);
                        if (h1 <= zero)
                        {
                            GetMinEdge12(a01, a11, b1, f10, f01, p);
                        }
                        else
                        {
                            GetMinInterior(p0, h0, p1, h1, p);
                        }
                    }
                }
            }
            else if (f10 <= zero)
            {
                // (5) p0 = (0,t10), p1 = (t01,1-t01),
                // H(z) = (t11 - t10)*G(L(z))
                p0[0] = zero;
                p0[1] = f00 / (f00 - f01);
                p1[0] = f01 / (f01 - f10);
                p1[1] = one - p1[0];
                dt1 = p1[1] - p0[1];
                h0 = dt1 * (a11 * p0[1] + b1);
                if (h0 >= zero)
                {
                    GetMinEdge02(a11, b1, p);
                }
                else
                {
                    h1 = dt1 * (a01 * p1[0] + a11 * p1[1] + b1);
                    if (h1 <= zero)
                    {
                        GetMinEdge12(a01, a11, b1, f10, f01, p);
                    }
                    else
                    {
                        GetMinInterior(p0, h0, p1, h1, p);
                    }
                }
            }
            else
            {
                // (6) p0 = (t00,0), p1 = (0,t11), H(z) = t11*G(L(z))
                p0[0] = f00 / (f00 - f10);
                p0[1] = zero;
                p1[0] = zero;
                p1[1] = f00 / (f00 - f01);
                h0 = p1[1] * (a01 * p0[0] + b1);
                if (h0 >= zero)
                {
                    p = p0;  // GetMinEdge01
                }
                else
                {
                    h1 = p1[1] * (a11 * p1[1] + b1);
                    if (h1 <= zero)
                    {
                        GetMinEdge02(a11, b1, p);
                    }
                    else
                    {
                        GetMinInterior(p0, h0, p1, h1, p);
                    }
                }
            }

            Result result{};
            result.closest[0] = point;
            result.closest[1] = triangle.v[0] + p[0] * edge0 + p[1] * edge1;
            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            result.barycentric[0] = one - p[0] - p[1];
            result.barycentric[1] = p[0];
            result.barycentric[2] = p[1];
            return result;
        }

    private:
        void GetMinEdge02(T const& a11, T const& b1, std::array<T, 2>& p)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            p[0] = zero;
            if (b1 >= zero)
            {
                p[1] = zero;
            }
            else if (a11 + b1 <= zero)
            {
                p[1] = one;
            }
            else
            {
                p[1] = -b1 / a11;
            }
        }

        inline void GetMinEdge12(T const& a01, T const& a11, T const& b1,
            T const& f10, T const& f01, std::array<T, 2>& p)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T h0 = a01 + b1 - f10;
            if (h0 >= zero)
            {
                p[1] = zero;
            }
            else
            {
                T h1 = a11 + b1 - f01;
                if (h1 <= zero)
                {
                    p[1] = one;
                }
                else
                {
                    p[1] = h0 / (h0 - h1);
                }
            }
            p[0] = one - p[1];
        }

        inline void GetMinInterior(std::array<T, 2> const& p0, T const& h0,
            std::array<T, 2> const& p1, T const& h1, std::array<T, 2>& p)
        {
            T z = h0 / (h0 - h1);
            T omz = static_cast<T>(1) - z;
            p[0] = omz * p0[0] + z * p1[0];
            p[1] = omz * p0[1] + z * p1[1];
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPPointTriangle = DCPQuery<T, Vector<N, T>, Triangle<N, T>>;

    template <typename T>
    using DCPPoint2Triangle2 = DCPPointTriangle<2, T>;

    template <typename T>
    using DCPPoint3Triangle3 = DCPPointTriangle<3, T>;
}
