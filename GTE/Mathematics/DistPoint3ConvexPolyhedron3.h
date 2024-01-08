// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a point and a convex polyhedron in 3D.  The
// algorithm is based on using an LCP solver for the convex quadratic
// programming problem.  For details, see
// https://www.geometrictools.com/Documentation/ConvexQuadraticProgramming.pdf

#include <Mathematics/DCPQuery.h>
#include <Mathematics/LCPSolver.h>
#include <Mathematics/ConvexPolyhedron3.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Vector3<T>, ConvexPolyhedron3<T>>
    {
    public:
        // Construction.  If you have no knowledge of the number of faces
        // for the convex polyhedra you plan on applying the query to, pass
        // 'numTriangles' of zero.  This is a request to the operator()
        // function to create the LCP solver for each query, and this
        // requires memory allocation and deallocation per query.  If you
        // plan on applying the query multiple/ times to a single polyhedron,
        // even if the vertices of the polyhedron are modified for each query,
        // then pass 'numTriangles' to be the number of triangle faces for
        // that polyhedron.  This lets the operator() function know to create
        // the LCP solver once at construction time, thus avoiding the memory
        // management costs during the query.
        DCPQuery(int32_t numTriangles = 0)
        {
            if (numTriangles > 0)
            {
                int32_t const n = numTriangles + 3;
                mLCP = std::make_unique<LCPSolver<T>>(n);
                mMaxLCPIterations = mLCP->GetMaxIterations();
            }
            else
            {
                mMaxLCPIterations = 0;
            }
        }

        struct Result
        {
            Result()
                :
                queryIsSuccessful(false),
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() },
                numLCPIterations(0)
            {
            }

            bool queryIsSuccessful;

            // These members are valid only when queryIsSuccessful is true;
            // otherwise, they are all set to zero.
            T distance, sqrDistance;
            std::array<Vector3<T>, 2> closest;

            // The number of iterations used by LCPSolver regardless of
            // whether the query is successful.
            int32_t numLCPIterations;
        };

        // Default maximum iterations is 144 (n = 12, maxIterations = n*n).
        // If the solver fails to converge, try increasing the maximum number
        // of iterations.
        void SetMaxLCPIterations(int32_t maxLCPIterations)
        {
            mMaxLCPIterations = maxLCPIterations;
            if (mLCP)
            {
                mLCP->SetMaxIterations(mMaxLCPIterations);
            }
        }

        Result operator()(Vector3<T> const& point, ConvexPolyhedron3<T> const& polyhedron)
        {
            Result result{};

            int32_t const numTriangles = static_cast<int32_t>(polyhedron.planes.size());
            if (numTriangles == 0)
            {
                // The polyhedron planes and aligned box need to be created.
                result.queryIsSuccessful = false;
                for (int32_t i = 0; i < 3; ++i)
                {
                    result.closest[0][i] = (T)0;
                    result.closest[1][i] = (T)0;
                }
                result.distance = (T)0;
                result.sqrDistance = (T)0;
                result.numLCPIterations = 0;
                return result;
            }

            int32_t const n = numTriangles + 3;

            // Translate the point and convex polyhedron so that the
            // polyhedron is in the first octant.  The translation is not
            // explicit; rather, the q and M for the LCP are initialized using
            // the translation information.
            Vector4<T> hmin = HLift(polyhedron.alignedBox.min, (T)1);

            std::vector<T> q(n);
            for (int32_t r = 0; r < 3; ++r)
            {
                q[r] = polyhedron.alignedBox.min[r] - point[r];
            }
            for (int32_t r = 3, t = 0; r < n; ++r, ++t)
            {
                q[r] = -Dot(polyhedron.planes[t], hmin);
            }

            std::vector<T> M(static_cast<size_t>(n) * static_cast<size_t>(n));
            M[0] = (T)1;
            M[1] = (T)0;
            M[2] = (T)0;
            M[n] = (T)0;
            M[static_cast<size_t>(n) + 1] = (T)1;
            M[static_cast<size_t>(n) + 2] = (T)0;
            M[2 * static_cast<size_t>(n)] = (T)0;
            M[2 * static_cast<size_t>(n) + 1] = (T)0;
            M[2 * static_cast<size_t>(n) + 2] = (T)1;
            for (int32_t t = 0, c = 3; t < numTriangles; ++t, ++c)
            {
                Vector3<T> normal = HProject(polyhedron.planes[t]);
                for (int32_t r = 0; r < 3; ++r)
                {
                    M[c + static_cast<size_t>(n) * r] = normal[r];
                    M[r + static_cast<size_t>(n) * c] = -normal[r];
                }
            }
            for (int32_t r = 3; r < n; ++r)
            {
                for (int32_t c = 3; c < n; ++c)
                {
                    M[c + static_cast<size_t>(n) * r] = (T)0;
                }
            }

            bool needsLCP = (mLCP == nullptr);
            if (needsLCP)
            {
                mLCP = std::make_unique<LCPSolver<T>>(n);
                if (mMaxLCPIterations > 0)
                {
                    mLCP->SetMaxIterations(mMaxLCPIterations);
                }
            }

            std::vector<T> w(n), z(n);
            if (mLCP->Solve(q, M, w, z))
            {
                result.queryIsSuccessful = true;
                result.closest[0] = point;
                for (int32_t i = 0; i < 3; ++i)
                {
                    result.closest[1][i] = z[i] + polyhedron.alignedBox.min[i];
                }

                Vector3<T> diff = result.closest[1] - result.closest[0];
                result.sqrDistance = Dot(diff, diff);
                result.distance = std::sqrt(result.sqrDistance);
            }
            else
            {
                // If you reach this case, the maximum number of iterations
                // was not specified to be large enough or there is a problem
                // due to floating-point rounding errors.  If you believe the
                // latter is true, file a bug report.
                result.queryIsSuccessful = false;
            }

            result.numLCPIterations = mLCP->GetNumIterations();
            if (needsLCP)
            {
                mLCP = nullptr;
            }
            return result;
        }

    private:
        int32_t mMaxLCPIterations;
        std::unique_ptr<LCPSolver<T>> mLCP;
    };
}
