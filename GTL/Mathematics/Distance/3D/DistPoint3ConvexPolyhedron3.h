// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a point and a convex polyhedron in 3D. The
// algorithm is based on using an LCP solver for the convex quadratic
// programming problem. For details, see
// https://www.geometrictools.com/Documentation/ConvexQuadraticProgramming.pdf

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/3D/ConvexPolyhedron3.h>
#include <GTL/Mathematics/Minimizers/LCPSolver.h>
#include <array>
#include <cmath>
#include <memory>
#include <vector>

// TODO: Remove the dependence on the LCP solver.

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Vector3<T>, ConvexPolyhedron3<T>>
    {
    public:
        // If you have no knowledge of the number of faces for the convex
        // polyhedra you plan on applying the query to, pass 'numTriangles' of
        // zero. This is a request to the operator() function to create the
        // LCP solver for each query, and this requires memory allocation and
        // deallocation per query. If you plan on applying the query multiple
        // times to a single polyhedron, even if the vertices of the
        // polyhedron are modified for each query, then pass 'numTriangles' to
        // be the number of triangle faces for that polyhedron.  This lets the
        // operator() function know to create the LCP solver once at
        // construction time, thus avoiding the memory management costs during
        // the query.
        DCPQuery(size_t numTriangles = 0)
            :
            mMaxLCPIterations(0),
            mLCP{}
        {
            if (numTriangles > 0)
            {
                size_t const n = numTriangles + 3;
                mLCP = std::make_unique<LCPSolver<T>>(n);
                mMaxLCPIterations = mLCP->GetMaxIterations();
            }
            else
            {
                mMaxLCPIterations = 0;
            }
        }

        // The input point is stored in the member closest[0]. The convex
        // polyhedron point closest to it is stored in the member closest[1].
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                closest{},
                numLCPIterations(0),
                queryIsSuccessful(false)
            {
            }

            // The distance, sqrDistance and closest[] memers are valid only
            // when queryIsSuccessful is true; otherwise, they are all set to
            // zero. The numLCPIterations is the number of iterations used by
            // the LCP solver, regardless whether the query is successful.
            T distance, sqrDistance;
            std::array<Vector3<T>, 2> closest;
            size_t numLCPIterations;
            bool queryIsSuccessful;
        };

        // Default maximum iterations is 144 (n = 12, maxIterations = n*n).
        // If the solver fails to converge, try increasing the maximum number
        // of iterations.
        void SetMaxLCPIterations(size_t maxLCPIterations)
        {
            mMaxLCPIterations = maxLCPIterations;
            if (mLCP)
            {
                mLCP->SetMaxIterations(mMaxLCPIterations);
            }
        }

        // The ConvexPolyhedron3<T> objects must have been created so that
        // planes of the faces and an axis-aligned bounding box of the
        // polyhedron are generated.
        //   size_t const numVertices = <some number>;
        //   size_t const numTriangles = <some number>;
        //   ConvexPolyhedron3<T> polyhedron{};
        //   polyhedron.vertices.resize(numVertices);
        //   polyhedron.indices.resize(3 * numTriangles);
        //   // Initialize the vertices and the indices here...
        //   polyhedron.GeneratePlanes();
        //   polyhedron.GenerateAlignedBox();
        // or
        //   std::vector<Vector3<T>> vertices(numVertices);
        //   std::vector<size_t> indices(3 * numTriangles);
        //   // Initialize the vertices and the indices here...
        //   ConvexPolyhedron3<T> polyhedron(
        //     std::move(vertices), std::move(indices), true, true);
        Output operator()(Vector3<T> const& point, ConvexPolyhedron3<T> const& polyhedron)
        {
            Output output{};

            size_t const numTriangles = polyhedron.planes.size();
            if (numTriangles == 0)
            {
                // The polyhedron planes and aligned box need to be created.
                output.queryIsSuccessful = false;
                for (size_t i = 0; i < 3; ++i)
                {
                    output.closest[0][i] = C_<T>(0);
                    output.closest[1][i] = C_<T>(0);
                }
                output.distance = C_<T>(0);
                output.sqrDistance = C_<T>(0);
                output.numLCPIterations = 0;
                return output;
            }

            size_t const n = numTriangles + 3;

            // Translate the point and convex polyhedron so that the
            // polyhedron is in the first octant. The translation is not
            // explicit; rather, the q and M for the LCP are initialized using
            // the translation information.
            Vector4<T> hmin = HLift(polyhedron.alignedBox.min, C_<T>(1));

            std::vector<T> q(n);
            for (size_t r = 0; r < 3; ++r)
            {
                q[r] = polyhedron.alignedBox.min[r] - point[r];
            }
            for (size_t r = 3, t = 0; r < n; ++r, ++t)
            {
                q[r] = -Dot(polyhedron.planes[t], hmin);
            }

            std::vector<T> M(n * n);
            M[0] = C_<T>(1);  M[1] = C_<T>(0);  M[2] = C_<T>(0);
            M[n] = C_<T>(0);  M[n + 1] = C_<T>(1);  M[n + 2] = C_<T>(0);
            M[2 * n] = C_<T>(0);  M[2 * n + 1] = C_<T>(0);  M[2 * n + 2] = C_<T>(1);
            for (size_t t = 0, c = 3; t < numTriangles; ++t, ++c)
            {
                Vector3<T> normal = HProject(polyhedron.planes[t]);
                for (size_t r = 0; r < 3; ++r)
                {
                    M[c + n * r] = normal[r];
                    M[r + n * c] = -normal[r];
                }
            }
            for (size_t r = 3; r < n; ++r)
            {
                for (size_t c = 3; c < n; ++c)
                {
                    M[c + n * r] = C_<T>(0);
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
            if ((*mLCP)(q, M, w, z))
            {
                output.queryIsSuccessful = true;
                output.closest[0] = point;
                for (size_t i = 0; i < 3; ++i)
                {
                    output.closest[1][i] = z[i] + polyhedron.alignedBox.min[i];
                }

                Vector3<T> diff = output.closest[1] - output.closest[0];
                output.sqrDistance = Dot(diff, diff);
                output.distance = std::sqrt(output.sqrDistance);
            }
            else
            {
                // If you reach this case, the maximum number of iterations
                // was not specified to be large enough or there is a problem
                // due to floating-point rounding errors. If you believe the
                // latter is true, file a bug report.
                output.queryIsSuccessful = false;
            }

            output.numLCPIterations = mLCP->GetNumIterations();
            if (needsLCP)
            {
                mLCP = nullptr;
            }
            return output;
        }

    private:
        size_t mMaxLCPIterations;
        std::unique_ptr<LCPSolver<T>> mLCP;
    };
}
