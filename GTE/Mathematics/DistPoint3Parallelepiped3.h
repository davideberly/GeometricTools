// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 7.2.2024.11.04

#pragma once

// Implementation of a point-parallelepiped distance and closest-point query.
// The details are described in
//   https://www.geometrictools.com/Documentation/DistancePointParallelpiped.pdf

#include <Mathematics/DistPoint2Parallelogram2.h>
#include <Mathematics/Matrix2x2.h>
#include <Mathematics/Matrix3x3.h>
#include "Parallelepiped3.h"
#include <array>
#include <cmath>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Vector3<T>, Parallelepiped3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            // The point closest[0] is the query point. The point closest[1]
            // is the parallelepiped point closest to the query point. The
            // two points are the same when the query point is contained by
            // the parallelepiped.
            T distance, sqrDistance;
            std::array<Vector3<T>, 2> closest;
        };

        Result operator()(Vector3<T> const& point, Parallelepiped3<T> const& ppd)
        {
            Result result{};

            // For a parallelepiped point X, let Y = {Dot(V0,X-C),Dot(V1,X-C),
            // Dot(V2,X-C)}. Compute the quadratic function q(Y) = (Y-Z)^T *
            // A * (Y-Z) / 2 where A = B^T * B is a symmetric matrix.
            Matrix3x3<T> B{};
            B.SetCol(0, ppd.axis[0]);
            B.SetCol(1, ppd.axis[1]);
            B.SetCol(2, ppd.axis[2]);
            Matrix3x3<T> A = MultiplyATB(B, B);

            // Transform the query point to parallelepiped coordinates,
            // Z = Inverse(B) * (P - C).
            Vector3<T> diff = point - ppd.center;
            Vector3<T> Z = Inverse(B) * diff;

            // Get the minimizer for q(Y).
            Vector3<T> K = GetMinimizer(A, Z);

            result.closest[0] = point;
            result.closest[1] = ppd.center + B * K;
            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

        Vector3<T> GetMinimizer(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K{};

            if (Z[2] < NegOne())
            {
                if (Z[1] < NegOne())
                {
                    if (Z[0] < NegOne())
                    {
                        K = GetClosestRmmm(A, Z);
                    }
                    else if (Z[0] <= PosOne())
                    {
                        K = GetClosestRzmm(A, Z);
                    }
                    else
                    {
                        K = GetClosestRpmm(A, Z);
                    }
                }
                else if (Z[1] <= PosOne())
                {
                    if (Z[0] < NegOne())
                    {
                        K = GetClosestRmzm(A, Z);
                    }
                    else if (Z[0] <= PosOne())
                    {
                        K = GetClosestRzzm(A, Z);
                    }
                    else
                    {
                        K = GetClosestRpzm(A, Z);
                    }
                }
                else
                {
                    if (Z[0] < NegOne())
                    {
                        K = GetClosestRmpm(A, Z);
                    }
                    else if (Z[0] <= PosOne())
                    {
                        K = GetClosestRzpm(A, Z);
                    }
                    else
                    {
                        K = GetClosestRppm(A, Z);
                    }
                }
            }
            else if (Z[2] <= PosOne())
            {
                if (Z[1] < NegOne())
                {
                    if (Z[0] < NegOne())
                    {
                        K = GetClosestRmmz(A, Z);
                    }
                    else if (Z[0] <= PosOne())
                    {
                        K = GetClosestRzmz(A, Z);
                    }
                    else
                    {
                        K = GetClosestRpmz(A, Z);
                    }
                }
                else if (Z[1] <= PosOne())
                {
                    if (Z[0] < NegOne())
                    {
                        K = GetClosestRmzz(A, Z);
                    }
                    else if (Z[0] <= PosOne())
                    {
                        K = GetClosestRzzz(A, Z);  // returns Z
                    }
                    else
                    {
                        K = GetClosestRpzz(A, Z);
                    }
                }
                else
                {
                    if (Z[0] < NegOne())
                    {
                        K = GetClosestRmpz(A, Z);
                    }
                    else if (Z[0] <= PosOne())
                    {
                        K = GetClosestRzpz(A, Z);
                    }
                    else
                    {
                        K = GetClosestRppz(A, Z);
                    }
                }
            }
            else
            {
                if (Z[1] < NegOne())
                {
                    if (Z[0] < NegOne())
                    {
                        K = GetClosestRmmp(A, Z);
                    }
                    else if (Z[0] <= PosOne())
                    {
                        K = GetClosestRzmp(A, Z);
                    }
                    else
                    {
                        K = GetClosestRpmp(A, Z);
                    }
                }
                else if (Z[1] <= PosOne())
                {
                    if (Z[0] < NegOne())
                    {
                        K = GetClosestRmzp(A, Z);
                    }
                    else if (Z[0] <= PosOne())
                    {
                        K = GetClosestRzzp(A, Z);
                    }
                    else
                    {
                        K = GetClosestRpzp(A, Z);
                    }
                }
                else
                {
                    if (Z[0] < NegOne())
                    {
                        K = GetClosestRmpp(A, Z);
                    }
                    else if (Z[0] <= PosOne())
                    {
                        K = GetClosestRzpp(A, Z);
                    }
                    else
                    {
                        K = GetClosestRppp(A, Z);
                    }
                }
            }

            return K;
        }

    private:
        static inline T NegOne() { return static_cast<T>(-1); }
        static inline T PosOne() { return static_cast<T>(+1); }

        using PPQuery = DCPQuery<T, Vector2<T>, Parallelogram2<T>>;

        inline Vector3<T> GetClosestRzzz(Matrix3x3<T> const&, Vector3<T> const& Z3)
        {
            return Z3;
        }

        Vector3<T> GetClosestRzzm(Matrix3x3<T> const& A3, Vector3<T> const& Z3)
        {
            Vector2<T> Z2 = { Z3[0], Z3[1] };
            T u = NegOne() - Z3[2];
            Matrix2x2<T> A2 = { A3(0,0), A3(0,1), A3(0,1), A3(1,1) };
            Vector2<T> V2 = { A3(0,2), A3(1,2) };
            Vector2<T> Zeta2 = Z2 - u * (Inverse(A2) * V2);
            Vector2<T> K2 = PPQuery{}.GetMinimizer(A2, Zeta2);
            Vector3<T> K3 = { K2[0], K2[1], NegOne() };
            return K3;
        }

        Vector3<T> GetClosestRzzp(Matrix3x3<T> const& A3, Vector3<T> const& Z3)
        {
            Vector2<T> Z2 = { Z3[0], Z3[1] };
            T u = PosOne() - Z3[2];
            Matrix2x2<T> A2 = { A3(0,0), A3(0,1), A3(0,1), A3(1,1) };
            Vector2<T> V2 = { A3(0,2), A3(1,2) };
            Vector2<T> Zeta2 = Z2 - u * (Inverse(A2) * V2);
            Vector2<T> K2 = PPQuery{}.GetMinimizer(A2, Zeta2);
            Vector3<T> K3 = { K2[0], K2[1], PosOne() };
            return K3;
        }

        Vector3<T> GetClosestRzmz(Matrix3x3<T> const& A3, Vector3<T> const& Z3)
        {
            Vector2<T> Z2 = { Z3[2], Z3[0] };
            T u = NegOne() - Z3[1];
            Matrix2x2<T> A2 = { A3(2,2), A3(0,2), A3(0,2), A3(0,0) };
            Vector2<T> V2 = { A3(1,2), A3(0,1) };
            Vector2<T> Zeta2 = Z2 - u * (Inverse(A2) * V2);
            Vector2<T> K2 = PPQuery{}.GetMinimizer(A2, Zeta2);
            Vector3<T> K3 = { K2[1], NegOne(), K2[0] };
            return K3;
        }

        Vector3<T> GetClosestRzpz(Matrix3x3<T> const& A3, Vector3<T> const& Z3)
        {
            Vector2<T> Z2 = { Z3[2], Z3[0] };
            T u = PosOne() - Z3[1];
            Matrix2x2<T> A2 = { A3(2,2), A3(0,2), A3(0,2), A3(0,0) };
            Vector2<T> V2 = { A3(1,2), A3(0,1) };
            Vector2<T> Zeta2 = Z2 - u * (Inverse(A2) * V2);
            Vector2<T> K2 = PPQuery{}.GetMinimizer(A2, Zeta2);
            Vector3<T> K3 = { K2[1], PosOne(), K2[0] };
            return K3;
        }

        Vector3<T> GetClosestRmzz(Matrix3x3<T> const& A3, Vector3<T> const& Z3)
        {
            Vector2<T> Z2 = { Z3[1], Z3[2] };
            T u = NegOne() - Z3[0];
            Matrix2x2<T> A2 = { A3(1,1), A3(1,2), A3(1,2), A3(2,2) };
            Vector2<T> V2 = { A3(0,1), A3(0,2) };
            Vector2<T> Zeta2 = Z2 - u * (Inverse(A2) * V2);
            Vector2<T> K2 = PPQuery{}.GetMinimizer(A2, Zeta2);
            Vector3<T> K3 = { NegOne(), K2[0], K2[1] };
            return K3;
        }

        Vector3<T> GetClosestRpzz(Matrix3x3<T> const& A3, Vector3<T> const& Z3)
        {
            Vector2<T> Z2 = { Z3[1], Z3[2] };
            T u = PosOne() - Z3[0];
            Matrix2x2<T> A2 = { A3(1,1), A3(1,2), A3(1,2), A3(2,2) };
            Vector2<T> V2 = { A3(0,1), A3(0,2) };
            Vector2<T> Zeta2 = Z2 - u * (Inverse(A2) * V2);
            Vector2<T> K2 = PPQuery{}.GetMinimizer(A2, Zeta2);
            Vector3<T> K3 = { PosOne(), K2[0], K2[1] };
            return K3;
        }

        Vector3<T> GetClosestRzmm(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K = GetClosestRzzm(A, Z);
            if (K[1] == NegOne())
            {
                K = GetClosestRzmz(A, Z);
            }
            return K;
        }

        Vector3<T> GetClosestRzmp(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K = GetClosestRzzp(A, Z);
            if (K[1] == NegOne())
            {
                K = GetClosestRzmz(A, Z);
            }
            return K;
        }

        Vector3<T> GetClosestRzpm(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K = GetClosestRzzm(A, Z);
            if (K[1] == PosOne())
            {
                K = GetClosestRzpz(A, Z);
            }
            return K;
        }

        Vector3<T> GetClosestRzpp(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K = GetClosestRzzp(A, Z);
            if (K[1] == PosOne())
            {
                K = GetClosestRzpz(A, Z);
            }
            return K;
        }

        Vector3<T> GetClosestRmzm(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K = GetClosestRzzm(A, Z);
            if (K[0] == NegOne())
            {
                K = GetClosestRmzz(A, Z);
            }
            return K;
        }

        Vector3<T> GetClosestRmzp(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K = GetClosestRzzp(A, Z);
            if (K[0] == NegOne())
            {
                K = GetClosestRmzz(A, Z);
            }
            return K;
        }

        Vector3<T> GetClosestRpzm(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K = GetClosestRzzm(A, Z);
            if (K[0] == PosOne())
            {
                K = GetClosestRpzz(A, Z);
            }
            return K;
        }

        Vector3<T> GetClosestRpzp(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K = GetClosestRzzp(A, Z);
            if (K[0] == PosOne())
            {
                K = GetClosestRpzz(A, Z);
            }
            return K;
        }

        Vector3<T> GetClosestRmmz(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K = GetClosestRmzz(A, Z);
            if (K[1] == NegOne())
            {
                K = GetClosestRzmz(A, Z);
            }
            return K;
        }

        Vector3<T> GetClosestRmpz(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K = GetClosestRmzz(A, Z);
            if (K[1] == PosOne())
            {
                K = GetClosestRzpz(A, Z);
            }
            return K;
        }

        Vector3<T> GetClosestRpmz(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K = GetClosestRpzz(A, Z);
            if (K[1] == NegOne())
            {
                K = GetClosestRzmz(A, Z);
            }
            return K;
        }

        Vector3<T> GetClosestRppz(Matrix3x3<T> const& A, Vector3<T> const& Z)
        {
            Vector3<T> K = GetClosestRpzz(A, Z);
            if (K[1] == PosOne())
            {
                K = GetClosestRzpz(A, Z);
            }
            return K;
        }

        Vector3<T> GetClosestRmmm(Matrix3x3<T> const& A, Vector3<T> const Z)
        {
            Vector3<T> K = GetClosestRzzm(A, Z);  // K[2] = -1
            if (K[1] == NegOne())
            {
                K = GetClosestRzmz(A, Z);  // K[1] = -1
                if (K[0] == NegOne())
                {
                    K = GetClosestRmzz(A, Z);  // K[0] = -1
                }
            }
            else if (K[0] == NegOne())
            {
                K = GetClosestRmzz(A, Z);  // K[0] = -1
                if (K[1] == NegOne())
                {
                    K = GetClosestRzmz(A, Z);  // K[1] = -1
                }
            }
            return K;
        }

        Vector3<T> GetClosestRmmp(Matrix3x3<T> const& A, Vector3<T> const Z)
        {
            Vector3<T> K = GetClosestRzzp(A, Z);  // K[2] = +1
            if (K[1] == NegOne())
            {
                K = GetClosestRzmz(A, Z);  // K[1] = -1
                if (K[0] == NegOne())
                {
                    K = GetClosestRmzz(A, Z);  // K[0] = -1
                }
            }
            else if (K[0] == NegOne())
            {
                K = GetClosestRmzz(A, Z);  // K[0] = -1
                if (K[1] == NegOne())
                {
                    K = GetClosestRzmz(A, Z);  // K[1] = -1
                }
            }
            return K;
        }

        Vector3<T> GetClosestRmpm(Matrix3x3<T> const& A, Vector3<T> const Z)
        {
            Vector3<T> K = GetClosestRzzm(A, Z);  // K[2] = -1
            if (K[1] == PosOne())
            {
                K = GetClosestRzpz(A, Z);  // K[1] = +1
                if (K[0] == NegOne())
                {
                    K = GetClosestRmzz(A, Z);  // K[0] = -1
                }
            }
            else if (K[0] == NegOne())
            {
                K = GetClosestRmzz(A, Z);  // K[0] = -1
                if (K[1] == PosOne())
                {
                    K = GetClosestRzpz(A, Z);  // K[1] = +1
                }
            }
            return K;
        }

        Vector3<T> GetClosestRmpp(Matrix3x3<T> const& A, Vector3<T> const Z)
        {
            Vector3<T> K = GetClosestRzzp(A, Z);  // K[2] = +1
            if (K[1] == PosOne())
            {
                K = GetClosestRzpz(A, Z);  // K[1] = +1
                if (K[0] == NegOne())
                {
                    K = GetClosestRmzz(A, Z);  // K[0] = -1
                }
            }
            else if (K[0] == NegOne())
            {
                K = GetClosestRmzz(A, Z);  // K[0] = -1
                if (K[1] == PosOne())
                {
                    K = GetClosestRzpz(A, Z);  // K[1] = +1
                }
            }
            return K;
        }

        Vector3<T> GetClosestRpmm(Matrix3x3<T> const& A, Vector3<T> const Z)
        {
            Vector3<T> K = GetClosestRzzm(A, Z);  // K[2] = -1
            if (K[1] == NegOne())
            {
                K = GetClosestRzmz(A, Z);  // K[1] = -1
                if (K[0] == PosOne())
                {
                    K = GetClosestRpzz(A, Z);  // K[0] = +1
                }
            }
            else if (K[0] == PosOne())
            {
                K = GetClosestRpzz(A, Z);  // K[0] = +1
                if (K[1] == NegOne())
                {
                    K = GetClosestRzmz(A, Z);  // K[1] = -1
                }
            }
            return K;
        }

        Vector3<T> GetClosestRpmp(Matrix3x3<T> const& A, Vector3<T> const Z)
        {
            Vector3<T> K = GetClosestRzzp(A, Z);  // K[2] = +1
            if (K[1] == NegOne())
            {
                K = GetClosestRzmz(A, Z);  // K[1] = -1
                if (K[0] == PosOne())
                {
                    K = GetClosestRpzz(A, Z);  // K[0] = -1
                }
            }
            else if (K[0] == PosOne())
            {
                K = GetClosestRpzz(A, Z);  // K[0] = +1
                if (K[1] == NegOne())
                {
                    K = GetClosestRzmz(A, Z);  // K[1] = -1
                }
            }
            return K;
        }

        Vector3<T> GetClosestRppm(Matrix3x3<T> const& A, Vector3<T> const Z)
        {
            Vector3<T> K = GetClosestRzzm(A, Z);  // K[2] = -1
            if (K[1] == PosOne())
            {
                K = GetClosestRzpz(A, Z);  // K[1] = +1
                if (K[0] == PosOne())
                {
                    K = GetClosestRpzz(A, Z);  // K[0] = +1
                }
            }
            else if (K[0] == PosOne())
            {
                K = GetClosestRpzz(A, Z);  // K[0] = +1
                if (K[1] == PosOne())
                {
                    K = GetClosestRzpz(A, Z);  // K[1] = +1
                }
            }
            return K;
        }

        Vector3<T> GetClosestRppp(Matrix3x3<T> const& A, Vector3<T> const Z)
        {
            Vector3<T> K = GetClosestRzzp(A, Z);  // K[2] = +1
            if (K[1] == PosOne())
            {
                K = GetClosestRzpz(A, Z);  // K[1] = +1
                if (K[0] == PosOne())
                {
                    K = GetClosestRpzz(A, Z);  // K[0] = +1
                }
            }
            else if (K[0] == PosOne())
            {
                K = GetClosestRpzz(A, Z);  // K[0] = +1
                if (K[1] == PosOne())
                {
                    K = GetClosestRzpz(A, Z);  // K[1] = +1
                }
            }
            return K;
        }
    };
}
