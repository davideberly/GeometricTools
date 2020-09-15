// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.01

#pragma once

// Compute the convex hull of 3D points using incremental insertion. The only
// way to ensure a correct result for the input vertices is to use an exact
// predicate for computing signs of various expressions. The implementation
// uses interval arithmetic and rational arithmetic for the predicate.

#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/ETManifoldMesh.h>
#include <Mathematics/Line.h>
#include <Mathematics/Hyperplane.h>
#include <Mathematics/Vector3.h>
#include <set>
#include <thread>

namespace gte
{
    template <typename Real>
    class ConvexHull3
    {
    public:
        // Supporting constants and types for rational arithmetic used in
        // the exact predicate for sign computations.
        static int constexpr NumWords = std::is_same<Real, float>::value ? 27 : 197;
        using Rational = BSNumber<UIntegerFP32<NumWords>>;

        // The class is a functor to support computing the convex hull of
        // multiple data sets using the same class object.
        ConvexHull3()
            :
            mEpsilon(static_cast<Real>(0)),
            mDimension(0),
            mLine(Vector3<Real>::Zero(), Vector3<Real>::Zero()),
            mPlane(Vector3<Real>::Zero(), static_cast<Real>(0)),
            mNumPoints(0),
            mNumUniquePoints(0),
            mPoints(nullptr)
        {
        }

        // The input is the array of points whose convex hull is required.
        // The epsilon value is used to determine the intrinsic dimensionality
        // of the vertices (d = 0, 1, 2, or 3). When epsilon is positive, the
        // determination is fuzzy where points approximately the same point,
        // approximately on a line, approximately planar or volumetric.
        bool operator()(int numPoints, Vector3<Real> const* points, Real epsilon)
        {
            mEpsilon = std::max(epsilon, static_cast<Real>(0));
            mDimension = 0;
            mLine.origin = Vector3<Real>::Zero();
            mLine.direction = Vector3<Real>::Zero();
            mPlane.normal = Vector3<Real>::Zero();
            mPlane.constant = (Real)0;
            mNumPoints = numPoints;
            mNumUniquePoints = 0;
            mPoints = points;
            mHullUnordered.clear();
            mHullMesh.Clear();

            if (mNumPoints < 4)
            {
                // ConvexHull3 should be called with at least four points.
                return false;
            }

            IntrinsicsVector3<Real> info(mNumPoints, mPoints, mEpsilon);
            if (info.dimension == 0)
            {
                // The set is (nearly) a point.
                return false;
            }

            if (info.dimension == 1)
            {
                // The set is (nearly) collinear.
                mDimension = 1;
                mLine = Line3<Real>(info.origin, info.direction[0]);
                return false;
            }

            if (info.dimension == 2)
            {
                // The set is (nearly) coplanar.
                mDimension = 2;
                mPlane = Plane3<Real>(UnitCross(info.direction[0],
                    info.direction[1]), info.origin);
                return false;
            }

            mDimension = 3;

            // Allocate storage for any rational points that must be
            // computed in the exact predicate.
            mRationalPoints.resize(mNumPoints);
            mConverted.resize(mNumPoints);
            std::fill(mConverted.begin(), mConverted.end(), 0u);

            // Insert the faces of the (nondegenerate) tetrahedron
            // constructed by the call to GetInformation.
            if (!info.extremeCCW)
            {
                std::swap(info.extreme[2], info.extreme[3]);
            }

            mHullUnordered.push_back(TriangleKey<true>(info.extreme[1],
                info.extreme[2], info.extreme[3]));
            mHullUnordered.push_back(TriangleKey<true>(info.extreme[0],
                info.extreme[3], info.extreme[2]));
            mHullUnordered.push_back(TriangleKey<true>(info.extreme[0],
                info.extreme[1], info.extreme[3]));
            mHullUnordered.push_back(TriangleKey<true>(info.extreme[0],
                info.extreme[2], info.extreme[1]));

            // Incrementally update the hull. The set of processed points is
            // maintained to eliminate duplicates, either in the original
            // input points or in the points obtained by snap rounding.
            std::set<Vector3<Real>> processed;
            for (int i = 0; i < 4; ++i)
            {
                processed.insert(points[info.extreme[i]]);
            }
            for (int i = 0; i < mNumPoints; ++i)
            {
                if (processed.find(points[i]) == processed.end())
                {
                    Update(i);
                    processed.insert(points[i]);
                }
            }
            mNumUniquePoints = static_cast<int>(processed.size());
            return true;
        }

        // Dimensional information. If GetDimension() returns 1, the points
        // lie on a line P+t*D (fuzzy comparison when epsilon > 0). You can
        // sort these if you need a polyline output by projecting onto the
        // line each vertex X = P+t*D, where t = Dot(D,X-P). If GetDimension()
        // returns 2, the points line on a plane P+s*U+t*V (fuzzy comparison
        // when epsilon > 0). You can project each point X = P+s*U+t*V, where
        // s = Dot(U,X-P) and t = Dot(V,X-P), then apply ConvexHull2 to the
        // (s,t) tuples.
        inline Real GetEpsilon() const
        {
            return mEpsilon;
        }

        inline int GetDimension() const
        {
            return mDimension;
        }

        inline Line3<Real> const& GetLine() const
        {
            return mLine;
        }

        inline Plane3<Real> const& GetPlane() const
        {
            return mPlane;
        }

        // Member access.
        inline int GetNumPoints() const
        {
            return mNumPoints;
        }

        inline int GetNumUniquePoints() const
        {
            return mNumUniquePoints;
        }

        inline Vector3<Real> const* GetPoints() const
        {
            return mPoints;
        }

        // The convex hull is a convex polyhedron with triangular faces.
        inline std::vector<TriangleKey<true>> const& GetHullUnordered() const
        {
            return mHullUnordered;
        }

        ETManifoldMesh const& GetHullMesh() const
        {
            // Create the mesh only on demand.
            if (mHullMesh.GetTriangles().size() == 0)
            {
                for (auto const& tri : mHullUnordered)
                {
                    mHullMesh.Insert(tri.V[0], tri.V[1], tri.V[2]);
                }
            }

            return mHullMesh;
        }

    private:
        // Support for incremental insertion.
        void Update(int i)
        {
            // The terminator that separates visible faces from nonvisible
            // faces is constructed by this code. Visible faces for the
            // incoming hull are removed, and the boundary of that set of
            // triangles is the terminator. New visible faces are added
            // using the incoming point and the edges of the terminator.
            //
            // A simple algorithm for computing terminator edges is the
            // following. Back-facing triangles are located and the three
            // edges are processed. The first time an edge is visited,
            // insert it into the terminator. If it is visited a second time,
            // the edge is removed because it is shared by another back-facing
            // triangle and, therefore, cannot be a terminator edge. After
            // visiting all back-facing triangles, the only remaining edges in
            // the map are the terminator edges.
            //
            // The order of vertices of an edge is important for adding new
            // faces with the correct vertex winding. However, the edge
            // "toggle" (insert edge, remove edge) should use edges with
            // unordered vertices, because the edge shared by one triangle has
            // opposite ordering relative to that of the other triangle. The
            // map uses unordered edges as the keys but stores the ordered
            // edge as the value. This avoids having to look up an edge twice
            // in a map with ordered edge keys.

            std::map<EdgeKey<false>, std::pair<int, int>> terminator;
            std::vector<TriangleKey<true>> backFaces;
            bool existsFrontFacingTriangle = false;
            for (auto const& tri : mHullUnordered)
            {
                int sign = ToPlane(i, tri.V[0], tri.V[1], tri.V[2]);
                if (sign <= 0)
                {
                    // The triangle is back facing.  These include triangles
                    // that are coplanar with the incoming point.
                    backFaces.push_back(tri);

                    // The current hull is a 2-manifold watertight mesh. The
                    // terminator edges are those shared with a front-facing
                    // triangle. The first time an edge of a back-facing
                    // triangle is visited, insert it into the terminator. If
                    // it is visited a second time, the edge is removed
                    // because it is shared by another back-facing triangle.
                    // After all back-facing triangles are visited, the only
                    // remaining edges are shared by a single back-facing
                    // triangle, which makes them terminator edges.
                    for (int j0 = 2, j1 = 0; j1 < 3; j0 = j1++)
                    {
                        int v0 = tri.V[j0], v1 = tri.V[j1];
                        EdgeKey<false> edge(v0, v1);
                        auto iter = terminator.find(edge);
                        if (iter == terminator.end())
                        {
                            // The edge is visited for the first time.
                            terminator.insert(std::make_pair(edge, std::make_pair(v0, v1)));
                        }
                        else
                        {
                            // The edge is visited for the second time.
                            terminator.erase(edge);
                        }
                    }
                }
                else
                {
                    // If there are no strictly front-facing triangles, then
                    // the incoming point is inside or on the convex hull. If
                    // we get to this code, then the point is truly outside
                    // and we can update the hull.
                    existsFrontFacingTriangle = true;
                }
            }

            if (!existsFrontFacingTriangle)
            {
                // The incoming point is inside or on the current hull, so no
                // update of the hull is necessary.
                return;
            }

            // The updated hull contains the triangles not visible to the
            // incoming point.
            mHullUnordered = backFaces;

            // Insert the triangles formed by the incoming point and the
            // terminator edges.
            for (auto const& edge : terminator)
            {
                mHullUnordered.push_back(TriangleKey<true>(i, edge.second.second, edge.second.first));
            }
        }

        // Memoized access to the rational representation of the points.
        Vector3<Rational> const& GetRationalPoint(int index) const
        {
            if (mConverted[index] == 0)
            {
                mConverted[index] = 1;
                for (int i = 0; i < 3; ++i)
                {
                    mRationalPoints[index][i] = mPoints[index][i];
                }
            }
            return mRationalPoints[index];
        }

        static Real IntervalProductDown(
            std::array<Real, 2> const& u, std::array<Real, 2> const& v)
        {
            Real const zero(0);
            Real w0;
            if (u[0] >= zero)
            {
                if (v[0] >= zero)
                {
                    w0 = u[0] * v[0];
                }
                else if (v[1] <= zero)
                {
                    w0 = u[1] * v[0];
                }
                else
                {
                    w0 = u[1] * v[0];
                }
            }
            else if (u[1] <= zero)
            {
                if (v[0] >= zero)
                {
                    w0 = u[0] * v[1];
                }
                else if (v[1] <= zero)
                {
                    w0 = u[1] * v[1];
                }
                else
                {
                    w0 = u[0] * v[1];
                }
            }
            else
            {
                if (v[0] >= zero)
                {
                    w0 = u[0] * v[1];
                }
                else if (v[1] <= zero)
                {
                    w0 = u[1] * v[0];
                }
                else
                {
                    w0 = u[0] * v[0];
                }
            }
            return w0;
        }

        static Real IntervalProductUp(
            std::array<Real, 2> const& u, std::array<Real, 2> const& v)
        {
            Real const zero(0);
            Real w1;
            if (u[0] >= zero)
            {
                if (v[0] >= zero)
                {
                    w1 = u[1] * v[1];
                }
                else if (v[1] <= zero)
                {
                    w1 = u[0] * v[1];
                }
                else
                {
                    w1 = u[1] * v[1];
                }
            }
            else if (u[1] <= zero)
            {
                if (v[0] >= zero)
                {
                    w1 = u[1] * v[0];
                }
                else if (v[1] <= zero)
                {
                    w1 = u[0] * v[0];
                }
                else
                {
                    w1 = u[0] * v[0];
                }
            }
            else
            {
                if (v[0] >= zero)
                {
                    w1 = u[1] * v[1];
                }
                else if (v[1] <= zero)
                {
                    w1 = u[0] * v[0];
                }
                else
                {
                    w1 = u[1] * v[1];
                }
            }
            return w1;
        }

        int ToPlane(int i, int v0, int v1, int v2) const
        {
            auto const& test = mPoints[i];
            auto const& vec0 = mPoints[v0];
            auto const& vec1 = mPoints[v1];
            auto const& vec2 = mPoints[v2];

            // Calling std::fesetround is expensive. To avoid calling it on
            // each interval operation, batch the round-down computations
            // and batch the round-up computations. Each contiguous block
            // has a round-down and a round-up subblock. The next block
            // consumes the results of both subblocks, so the setting of the
            // rounding mode must occur multiple times.
            auto saveMode = std::fegetround();
            std::array<Real, 2> x0, y0, z0, x1, y1, z1, x2, y2, z2;
            std::fesetround(FE_DOWNWARD);
            x0[0] = test[0] - vec0[0];
            y0[0] = test[1] - vec0[1];
            z0[0] = test[2] - vec0[2];
            x1[0] = vec1[0] - vec0[0];
            y1[0] = vec1[1] - vec0[1];
            z1[0] = vec1[2] - vec0[2];
            x2[0] = vec2[0] - vec0[0];
            y2[0] = vec2[1] - vec0[1];
            z2[0] = vec2[2] - vec0[2];
            std::fesetround(FE_UPWARD);
            x0[1] = test[0] - vec0[0];
            y0[1] = test[1] - vec0[1];
            z0[1] = test[2] - vec0[2];
            x1[1] = vec1[0] - vec0[0];
            y1[1] = vec1[1] - vec0[1];
            z1[1] = vec1[2] - vec0[2];
            x2[1] = vec2[0] - vec0[0];
            y2[1] = vec2[1] - vec0[1];
            z2[1] = vec2[2] - vec0[2];

            std::array<Real, 2> y1z2, y2z1, y2z0, y0z2, y0z1, y1z0;
            std::fesetround(FE_DOWNWARD);
            y1z2[0] = IntervalProductDown(y1, z2);
            y2z1[0] = IntervalProductDown(y2, z1);
            y2z0[0] = IntervalProductDown(y2, z0);
            y0z2[0] = IntervalProductDown(y0, z2);
            y0z1[0] = IntervalProductDown(y0, z1);
            y1z0[0] = IntervalProductDown(y1, z0);
            std::fesetround(FE_UPWARD);
            y1z2[1] = IntervalProductUp(y1, z2);
            y2z1[1] = IntervalProductUp(y2, z1);
            y2z0[1] = IntervalProductUp(y2, z0);
            y0z2[1] = IntervalProductUp(y0, z2);
            y0z1[1] = IntervalProductUp(y0, z1);
            y1z0[1] = IntervalProductUp(y1, z0);

            std::array<Real, 2> c0, c1, c2;
            std::fesetround(FE_DOWNWARD);
            c0[0] = y1z2[0] - y2z1[1];
            c1[0] = y2z0[0] - y0z2[1];
            c2[0] = y0z1[0] - y1z0[1];
            std::fesetround(FE_UPWARD);
            c0[1] = y1z2[1] - y2z1[0];
            c1[1] = y2z0[1] - y0z2[0];
            c2[1] = y0z1[1] - y1z0[0];

            std::array<Real, 2> x0c0, x1c1, x2c2, det;
            std::fesetround(FE_DOWNWARD);
            x0c0[0] = IntervalProductDown(x0, c0);
            x1c1[0] = IntervalProductDown(x1, c1);
            x2c2[0] = IntervalProductDown(x2, c2);
            det[0] = x0c0[0] + x1c1[0] + x2c2[0];
            std::fesetround(FE_UPWARD);
            x0c0[1] = IntervalProductUp(x0, c0);
            x1c1[1] = IntervalProductUp(x1, c1);
            x2c2[1] = IntervalProductUp(x2, c2);
            det[1] = x0c0[1] + x1c1[1] + x2c2[1];
            std::fesetround(saveMode);

            Real const zero = static_cast<Real>(0);
            int sign;
            if (det[0] > zero)
            {
                sign = +1;
            }
            else if (det[1] < zero)
            {
                sign = -1;
            }
            else
            {
                // The exact sign of the determinant is not known, so compute
                // the determinant using rational arithmetic.
                auto const& rtest = GetRationalPoint(i);
                auto const& rvec0 = GetRationalPoint(v0);
                auto const& rvec1 = GetRationalPoint(v1);
                auto const& rvec2 = GetRationalPoint(v2);
                auto rdiff0 = rtest - rvec0;
                auto rdiff1 = rvec1 - rvec0;
                auto rdiff2 = rvec2 - rvec0;
                auto rdet = DotCross(rdiff0, rdiff1, rdiff2);
                sign = rdet.GetSign();
            }

            return sign;
        }

        // The epsilon value is used for fuzzy determination of intrinsic
        // dimensionality. If the dimension is 0, 1, or 2, the constructor
        // returns early. The caller is responsible for retrieving the
        // dimension and taking an alternate path should the dimension be
        // smaller than 3. If the dimension is 0, the caller may as well
        // treat all points[] as a single point, say, points[0]. If the
        // dimension is 1, the caller can query for the approximating line
        // and project points[] onto it for further processing. If the
        // dimension is 2, the caller can query for the approximating plane
        // and project points[] onto it for further processing.
        Real mEpsilon;
        int mDimension;
        Line3<Real> mLine;
        Plane3<Real> mPlane;

        // The array of rational points used for the exact predicate. The
        // mConverted array is used to store 0 or 1, where initially the
        // values are 0. The first time mComputePoints[i] is encountered,
        // mConverted[i] is 0. The floating-point vector is converted to
        // a rational number, after which mConverted[1] is set to 1 to
        // avoid converting again if the floating-point vector is
        // encountered in another predicate computation.
        mutable std::vector<Vector3<Rational>> mRationalPoints;
        mutable std::vector<uint32_t> mConverted;

        int mNumPoints;
        int mNumUniquePoints;
        Vector3<Real> const* mPoints;
        std::vector<TriangleKey<true>> mHullUnordered;
        mutable ETManifoldMesh mHullMesh;
        uint32_t mNumThreads;
    };
}
