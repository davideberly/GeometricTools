// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.07.17

#pragma once

// Delaunay tetrahedralization of points (intrinsic dimensionality 3).
//   VQ = number of vertices
//   V  = array of vertices
//   TQ = number of tetrahedra
//   I  = Array of 4-tuples of indices into V that represent the tetrahedra
//        (4*TQ total elements).  Access via GetIndices(*).
//   A  = Array of 4-tuples of indices into I that represent the adjacent
//        tetrahedra (4*TQ total elements).  Access via GetAdjacencies(*).
// The i-th tetrahedron has vertices
//   vertex[0] = V[I[4*i+0]]
//   vertex[1] = V[I[4*i+1]]
//   vertex[2] = V[I[4*i+2]]
//   vertex[3] = V[I[4*i+3]]
// and face index triples listed below.  The face vertex ordering when
// viewed from outside the tetrahedron is counterclockwise.
//   face[0] = <I[4*i+1],I[4*i+2],I[4*i+3]>
//   face[1] = <I[4*i+0],I[4*i+3],I[4*i+2]>
//   face[2] = <I[4*i+0],I[4*i+1],I[4*i+3]>
//   face[3] = <I[4*i+0],I[4*i+2],I[4*i+1]>
// The tetrahedra adjacent to these faces have indices
//   adjacent[0] = A[4*i+0] is the tetrahedron opposite vertex[0], so it
//                 is the tetrahedron sharing face[0].
//   adjacent[1] = A[4*i+1] is the tetrahedron opposite vertex[1], so it
//                 is the tetrahedron sharing face[1].
//   adjacent[2] = A[4*i+2] is the tetrahedron opposite vertex[2], so it
//                 is the tetrahedron sharing face[2].
//   adjacent[3] = A[4*i+3] is the tetrahedron opposite vertex[3], so it
//                 is the tetrahedron sharing face[3].
// If there is no adjacent tetrahedron, the A[*] value is set to -1.  The
// tetrahedron adjacent to face[j] has vertices
//   adjvertex[0] = V[I[4*adjacent[j]+0]]
//   adjvertex[1] = V[I[4*adjacent[j]+1]]
//   adjvertex[2] = V[I[4*adjacent[j]+2]]
//   adjvertex[3] = V[I[4*adjacent[j]+3]]
// The only way to ensure a correct result for the input vertices (assumed to
// be exact) is to choose ComputeType for exact rational arithmetic.  You may
// use BSNumber.  No divisions are performed in this computation, so you do
// not have to use BSRational.

#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Logger.h>
#include <Mathematics/PrimalQuery3.h>
#include <Mathematics/TSManifoldMesh.h>
#include <Mathematics/Line.h>
#include <Mathematics/Hyperplane.h>
#include <Mathematics/SWInterval.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <numeric>
#include <set>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace gte
{
    // The variadic template declaration supports the class
    // Delaunay3<InputType, ComputeType>, which is deprecated and will be
    // removed in a future release. The declaration also supports the
    // replacement class Delaunay3<InputType>. The new class uses a blend of
    // interval arithmetic and rational arithmetic. It also uses unordered
    // sets (hash tables). The replacement performs much better than the
    // deprecated class.
    template <typename T, typename...>
    class Delaunay3 {};
}

namespace gte
{
    // This class requires you to specify the ComputeType yourself. If it
    // is BSNumber<> or BSRational<>, the worst-case choices of N for the
    // chosen InputType are listed in the next table. The numerical
    // computations are encapsulated in PrimalQuery3<ComputeType>::ToPlane and
    // PrimalQuery3<ComputeType>::ToCircumsphere, the latter query the
    // dominant one in determining N. We recommend using only BSNumber,
    // because no divisions are performed in the triangulation computations.
    //
    //    input type | compute type | N
    //    -----------+--------------+------
    //    float      | BSNumber     |    44
    //    double     | BSNumber     |   329
    //    float      | BSRational   |  1875
    //    double     | BSRational   | 14167

    template <typename InputType, typename ComputeType>
    class  // [[deprecated("Use Delaunay3<T> instead.")]]
        Delaunay3<InputType, ComputeType>
    {
    public:
        // The class is a functor to support computing the Delaunay
        // tetrahedralization of multiple data sets using the same class
        // object.
        Delaunay3()
            :
            mEpsilon((InputType)0),
            mDimension(0),
            mLine(Vector3<InputType>::Zero(), Vector3<InputType>::Zero()),
            mPlane(Vector3<InputType>::Zero(), (InputType)0),
            mNumVertices(0),
            mNumUniqueVertices(0),
            mNumTetrahedra(0),
            mVertices(nullptr)
        {
        }

        virtual ~Delaunay3() = default;

        // The input is the array of vertices whose Delaunay
        // tetrahedralization is required.  The epsilon value is used to
        // determine the intrinsic dimensionality of the vertices
        // (d = 0, 1, 2, or 3).  When epsilon is positive, the determination
        // is fuzzy--vertices approximately the same point, approximately on
        // a line, approximately planar, or volumetric.
        bool operator()(int32_t numVertices, Vector3<InputType> const* vertices, InputType epsilon)
        {
            mEpsilon = std::max(epsilon, (InputType)0);
            mDimension = 0;
            mLine.origin = Vector3<InputType>::Zero();
            mLine.direction = Vector3<InputType>::Zero();
            mPlane.normal = Vector3<InputType>::Zero();
            mPlane.constant = (InputType)0;
            mNumVertices = numVertices;
            mNumUniqueVertices = 0;
            mNumTetrahedra = 0;
            mVertices = vertices;
            mGraph.Clear();
            mIndices.clear();
            mAdjacencies.clear();

            int32_t i, j;
            if (mNumVertices < 4)
            {
                // Delaunay3 should be called with at least four points.
                return false;
            }

            IntrinsicsVector3<InputType> info(mNumVertices, vertices, mEpsilon);
            if (info.dimension == 0)
            {
                // mDimension is 0; mGraph, mIndices, and mAdjacencies are
                // empty
                return false;
            }

            if (info.dimension == 1)
            {
                // The set is (nearly) collinear.
                mDimension = 1;
                mLine = Line3<InputType>(info.origin, info.direction[0]);
                return false;
            }

            if (info.dimension == 2)
            {
                // The set is (nearly) coplanar.
                mDimension = 2;
                mPlane = Plane3<InputType>(UnitCross(info.direction[0],
                    info.direction[1]), info.origin);
                return false;
            }

            mDimension = 3;

            // Compute the vertices for the queries.
            mComputeVertices.resize(mNumVertices);
            mQuery.Set(mNumVertices, &mComputeVertices[0]);
            for (i = 0; i < mNumVertices; ++i)
            {
                for (j = 0; j < 3; ++j)
                {
                    mComputeVertices[i][j] = vertices[i][j];
                }
            }

            // Insert the (nondegenerate) tetrahedron constructed by the call
            // to GetInformation. This is necessary for the circumsphere
            // visibility algorithm to work correctly.
            if (!info.extremeCCW)
            {
                std::swap(info.extreme[2], info.extreme[3]);
            }
            if (!mGraph.Insert(info.extreme[0], info.extreme[1], info.extreme[2], info.extreme[3]))
            {
                return false;
            }

            // Incrementally update the tetrahedralization.  The set of
            // processed points is maintained to eliminate duplicates, either
            // in the original input points or in the points obtained by snap
            // rounding.
            std::set<Vector3<InputType>> processed;
            for (i = 0; i < 4; ++i)
            {
                processed.insert(vertices[info.extreme[i]]);
            }
            for (i = 0; i < mNumVertices; ++i)
            {
                if (processed.find(vertices[i]) == processed.end())
                {
                    if (!Update(i))
                    {
                        // A failure can occur if ComputeType is not an exact
                        // arithmetic type.
                        return false;
                    }
                    processed.insert(vertices[i]);
                }
            }
            mNumUniqueVertices = static_cast<int32_t>(processed.size());

            // Assign integer values to the tetrahedra for use by the caller
            // and copy the tetrahedra information to compact arrays mIndices
            // and mAdjacencies.
            UpdateIndicesAdjacencies();

            return true;
        }

        // Dimensional information.  If GetDimension() returns 1, the points
        // lie on a line P+t*D (fuzzy comparison when epsilon > 0).  You can
        // sort these if you need a polyline output by projecting onto the
        // line each vertex X = P+t*D, where t = Dot(D,X-P).  If
        // GetDimension() returns 2, the points line on a plane P+s*U+t*V
        // (fuzzy comparison when epsilon > 0).  You can project each vertex
        // X = P+s*U+t*V, where s = Dot(U,X-P) and t = Dot(V,X-P), then apply
        // Delaunay2 to the (s,t) tuples.
        inline InputType GetEpsilon() const
        {
            return mEpsilon;
        }

        inline int32_t GetDimension() const
        {
            return mDimension;
        }

        inline Line3<InputType> const& GetLine() const
        {
            return mLine;
        }

        inline Plane3<InputType> const& GetPlane() const
        {
            return mPlane;
        }

        // Member access.
        inline int32_t GetNumVertices() const
        {
            return mNumVertices;
        }

        inline int32_t GetNumUniqueVertices() const
        {
            return mNumUniqueVertices;
        }

        inline int32_t GetNumTetrahedra() const
        {
            return mNumTetrahedra;
        }

        inline Vector3<InputType> const* GetVertices() const
        {
            return mVertices;
        }

        inline PrimalQuery3<ComputeType> const& GetQuery() const
        {
            return mQuery;
        }

        inline TSManifoldMesh const& GetGraph() const
        {
            return mGraph;
        }

        inline std::vector<int32_t> const& GetIndices() const
        {
            return mIndices;
        }

        inline std::vector<int32_t> const& GetAdjacencies() const
        {
            return mAdjacencies;
        }

        // Locate those tetrahedra faces that do not share other tetrahedra.
        // The returned array has hull.size() = 3*numFaces indices, each
        // triple representing a triangle.  The triangles are counterclockwise
        // ordered when viewed from outside the hull.  The return value is
        // 'true' iff the dimension is 3.
        bool GetHull(std::vector<int32_t>& hull) const
        {
            if (mDimension == 3)
            {
                // Count the number of triangles that are not shared by two
                // tetrahedra.
                int32_t numTriangles = 0;
                for (auto adj : mAdjacencies)
                {
                    if (adj == -1)
                    {
                        ++numTriangles;
                    }
                }

                if (numTriangles > 0)
                {
                    // Enumerate the triangles.  The prototypical case is the
                    // single tetrahedron V[0] = (0,0,0), V[1] = (1,0,0),
                    // V[2] = (0,1,0) and V[3] = (0,0,1) with no adjacent
                    // tetrahedra.  The mIndices[] array is <0,1,2,3>.
                    //   i = 0, face = 0:
                    //    skip index 0, <x,1,2,3>, no swap, triangle = <1,2,3>
                    //   i = 1, face = 1:
                    //    skip index 1, <0,x,2,3>, swap,    triangle = <0,3,2>
                    //   i = 2, face = 2:
                    //    skip index 2, <0,1,x,3>, no swap, triangle = <0,1,3>
                    //   i = 3, face = 3:
                    //    skip index 3, <0,1,2,x>, swap,    triangle = <0,2,1>
                    // To guarantee counterclockwise order of triangles when
                    // viewed outside the tetrahedron, the swap of the last
                    // two indices occurs when face is an odd number;
                    // (face % 2) != 0.
                    hull.resize(3 * static_cast<size_t>(numTriangles));
                    size_t current = 0, i = 0;
                    for (auto adj : mAdjacencies)
                    {
                        if (adj == -1)
                        {
                            size_t tetra = i / 4, face = i % 4;
                            for (size_t j = 0; j < 4; ++j)
                            {
                                if (j != face)
                                {
                                    hull[current++] = mIndices[4 * tetra + j];
                                }
                            }
                            if ((face % 2) != 0)
                            {
                                std::swap(hull[current - 1], hull[current - 2]);
                            }
                        }
                        ++i;
                    }
                    return true;
                }
                else
                {
                    LogError("Unexpected. There must be at least one tetrahedron.");
                }
            }
            else
            {
                LogError("The dimension must be 3.");
            }
        }

        // Copy Delaunay tetrahedra to compact arrays mIndices and
        // mAdjacencies. The array information is accessible via the
        // functions GetIndices(int32_t, std::array<int32_t, 4>&) and
        // GetAdjacencies(int32_t, std::array<int32_t, 4>&).
        void UpdateIndicesAdjacencies()
        {
            // Assign integer values to the tetrahedra for use by the caller.
            auto const& smap = mGraph.GetTetrahedra();
            std::map<Tetrahedron*, int32_t> permute;
            int32_t i = -1;
            permute[nullptr] = i++;
            for (auto const& element : smap)
            {
                permute[element.second.get()] = i++;
            }

            // Put Delaunay tetrahedra into an array (vertices and adjacency
            // info).
            mNumTetrahedra = static_cast<int32_t>(smap.size());
            int32_t numIndices = 4 * mNumTetrahedra;
            if (mNumTetrahedra > 0)
            {
                mIndices.resize(numIndices);
                mAdjacencies.resize(numIndices);
                i = 0;
                for (auto const& element : smap)
                {
                    Tetrahedron* tetra = element.second.get();
                    for (size_t j = 0; j < 4; ++j, ++i)
                    {
                        mIndices[i] = tetra->V[j];
                        mAdjacencies[i] = permute[tetra->S[j]];
                    }
                }
            }
        }

        // Get the vertex indices for tetrahedron i.  The function returns
        // 'true' when the dimension is 3 and i is a valid tetrahedron index,
        // in which case the vertices are valid; otherwise, the function
        // returns 'false' and the vertices are invalid.
        bool GetIndices(int32_t i, std::array<int32_t, 4>& indices) const
        {
            if (mDimension == 3)
            {
                int32_t numTetrahedra = static_cast<int32_t>(mIndices.size() / 4);
                if (0 <= i && i < numTetrahedra)
                {
                    size_t fourI = 4 * static_cast<size_t>(i);
                    indices[0] = mIndices[fourI];
                    indices[1] = mIndices[fourI + 1];
                    indices[2] = mIndices[fourI + 2];
                    indices[3] = mIndices[fourI + 3];
                    return true;
                }
            }
            else
            {
                LogError("The dimension must be 3.");
            }
            return false;
        }

        // Get the indices for tetrahedra adjacent to tetrahedron i.  The
        // function returns 'true' when the dimension is 3 and i is a valid
        // tetrahedron index, in which case the adjacencies are valid;
        // otherwise, the function returns 'false' and the adjacencies are
        // invalid.
        bool GetAdjacencies(int32_t i, std::array<int32_t, 4>& adjacencies) const
        {
            if (mDimension == 3)
            {
                int32_t numTetrahedra = static_cast<int32_t>(mIndices.size() / 4);
                if (0 <= i && i < numTetrahedra)
                {
                    size_t fourI = 4 * static_cast<size_t>(i);
                    adjacencies[0] = mAdjacencies[fourI];
                    adjacencies[1] = mAdjacencies[fourI + 1];
                    adjacencies[2] = mAdjacencies[fourI + 2];
                    adjacencies[3] = mAdjacencies[fourI + 3];
                    return true;
                }
            }
            else
            {
                LogError("The dimension must be 3.");
            }
            return false;
        }

        // Support for searching the tetrahedralization for a tetrahedron
        // that contains a point.  If there is a containing tetrahedron, the
        // returned value is a tetrahedron index i with
        // 0 <= i < GetNumTetrahedra().  If there is not a containing
        // tetrahedron, -1 is returned.  The computations are performed using
        // exact rational arithmetic.
        //
        // The SearchInfo input stores information about the tetrahedron
        // search when looking for the tetrahedron (if any) that contains p.
        // The first tetrahedron searched is 'initialTetrahedron'.  On return
        // 'path' stores those (ordered) tetrahedron indices visited during
        // the search.  The last visited tetrahedron has index
        // 'finalTetrahedron' and vertex indices 'finalV[0,1,2,3]', stored in
        // volumetric counterclockwise order.  The last face of the search is
        // <finalV[0],finalV[1],finalV[2]>.  For spatially coherent inputs p
        // for numerous calls to this function, you will want to specify
        // 'finalTetrahedron' from the previous call as 'initialTetrahedron'
        // for the next call, which should reduce search times.
        struct SearchInfo
        {
            SearchInfo()
                :
                initialTetrahedron(0),
                numPath(0),
                path{},
                finalTetrahedron(0),
                finalV{ 0, 0, 0, 0 }
            {
            }

            int32_t initialTetrahedron;
            int32_t numPath;
            std::vector<int32_t> path;
            int32_t finalTetrahedron;
            std::array<int32_t, 4> finalV;
        };

        int32_t GetContainingTetrahedron(Vector3<InputType> const& p, SearchInfo& info) const
        {
            if (mDimension == 3)
            {
                Vector3<ComputeType> test{ p[0], p[1], p[2] };

                int32_t numTetrahedra = static_cast<int32_t>(mIndices.size() / 4);
                info.path.resize(numTetrahedra);
                info.numPath = 0;
                int32_t tetrahedron;
                if (0 <= info.initialTetrahedron
                    && info.initialTetrahedron < numTetrahedra)
                {
                    tetrahedron = info.initialTetrahedron;
                }
                else
                {
                    info.initialTetrahedron = 0;
                    tetrahedron = 0;
                }

                // Use tetrahedron faces as binary separating planes.
                for (int32_t i = 0; i < numTetrahedra; ++i)
                {
                    int32_t ibase = 4 * tetrahedron;
                    int32_t const* v = &mIndices[ibase];

                    info.path[info.numPath++] = tetrahedron;
                    info.finalTetrahedron = tetrahedron;
                    info.finalV[0] = v[0];
                    info.finalV[1] = v[1];
                    info.finalV[2] = v[2];
                    info.finalV[3] = v[3];

                    // <V1,V2,V3> counterclockwise when viewed outside
                    // tetrahedron.
                    if (mQuery.ToPlane(test, v[1], v[2], v[3]) > 0)
                    {
                        tetrahedron = mAdjacencies[ibase];
                        if (tetrahedron == -1)
                        {
                            info.finalV[0] = v[1];
                            info.finalV[1] = v[2];
                            info.finalV[2] = v[3];
                            info.finalV[3] = v[0];
                            return -1;
                        }
                        continue;
                    }

                    // <V0,V3,V2> counterclockwise when viewed outside
                    // tetrahedron.
                    if (mQuery.ToPlane(test, v[0], v[2], v[3]) < 0)
                    {
                        tetrahedron = mAdjacencies[static_cast<size_t>(ibase) + 1];
                        if (tetrahedron == -1)
                        {
                            info.finalV[0] = v[0];
                            info.finalV[1] = v[2];
                            info.finalV[2] = v[3];
                            info.finalV[3] = v[1];
                            return -1;
                        }
                        continue;
                    }

                    // <V0,V1,V3> counterclockwise when viewed outside
                    // tetrahedron.
                    if (mQuery.ToPlane(test, v[0], v[1], v[3]) > 0)
                    {
                        tetrahedron = mAdjacencies[static_cast<size_t>(ibase) + 2];
                        if (tetrahedron == -1)
                        {
                            info.finalV[0] = v[0];
                            info.finalV[1] = v[1];
                            info.finalV[2] = v[3];
                            info.finalV[3] = v[2];
                            return -1;
                        }
                        continue;
                    }

                    // <V0,V2,V1> counterclockwise when viewed outside
                    // tetrahedron.
                    if (mQuery.ToPlane(test, v[0], v[1], v[2]) < 0)
                    {
                        tetrahedron = mAdjacencies[static_cast<size_t>(ibase) + 3];
                        if (tetrahedron == -1)
                        {
                            info.finalV[0] = v[0];
                            info.finalV[1] = v[1];
                            info.finalV[2] = v[2];
                            info.finalV[3] = v[3];
                            return -1;
                        }
                        continue;
                    }

                    return tetrahedron;
                }
            }
            else
            {
                LogError("The dimension must be 3.");
            }
            return -1;
        }

    private:
        // Support for incremental Delaunay tetrahedralization.
        typedef TSManifoldMesh::Tetrahedron Tetrahedron;

        bool GetContainingTetrahedron(int32_t i, Tetrahedron*& tetra) const
        {
            int32_t numTetrahedra = static_cast<int32_t>(mGraph.GetTetrahedra().size());
            for (int32_t t = 0; t < numTetrahedra; ++t)
            {
                int32_t j;
                for (j = 0; j < 4; ++j)
                {
                    auto const& opposite = TetrahedronKey<true>::GetOppositeFace();
                    int32_t v0 = tetra->V[opposite[j][0]];
                    int32_t v1 = tetra->V[opposite[j][1]];
                    int32_t v2 = tetra->V[opposite[j][2]];
                    if (mQuery.ToPlane(i, v0, v1, v2) > 0)
                    {
                        // Point i sees face <v0,v1,v2> from outside the
                        // tetrahedron.
                        auto adjTetra = tetra->S[j];
                        if (adjTetra)
                        {
                            // Traverse to the tetrahedron sharing the face.
                            tetra = adjTetra;
                            break;
                        }
                        else
                        {
                            // We reached a hull face, so the point is outside
                            // the hull.
                            return false;
                        }
                    }

                }

                if (j == 4)
                {
                    // The point is inside all four faces, so the point is inside
                    // a tetrahedron.
                    return true;
                }
            }

            LogError("Unexpected termination of loop.");
        }

        bool GetAndRemoveInsertionPolyhedron(int32_t i, std::set<Tetrahedron*>& candidates,
            std::set<TriangleKey<true>>& boundary)
        {
            // Locate the tetrahedra that make up the insertion polyhedron.
            TSManifoldMesh polyhedron;
            while (candidates.size() > 0)
            {
                Tetrahedron* tetra = *candidates.begin();
                candidates.erase(candidates.begin());

                for (int32_t j = 0; j < 4; ++j)
                {
                    auto adj = tetra->S[j];
                    if (adj && candidates.find(adj) == candidates.end())
                    {
                        int32_t a0 = adj->V[0];
                        int32_t a1 = adj->V[1];
                        int32_t a2 = adj->V[2];
                        int32_t a3 = adj->V[3];
                        if (mQuery.ToCircumsphere(i, a0, a1, a2, a3) <= 0)
                        {
                            // Point i is in the circumsphere.
                            candidates.insert(adj);
                        }
                    }
                }

                int32_t v0 = tetra->V[0];
                int32_t v1 = tetra->V[1];
                int32_t v2 = tetra->V[2];
                int32_t v3 = tetra->V[3];
                if (!polyhedron.Insert(v0, v1, v2, v3))
                {
                    return false;
                }
                if (!mGraph.Remove(v0, v1, v2, v3))
                {
                    return false;
                }
            }

            // Get the boundary triangles of the insertion polyhedron.
            for (auto const& element : polyhedron.GetTetrahedra())
            {
                Tetrahedron* tetra = element.second.get();
                for (int32_t j = 0; j < 4; ++j)
                {
                    if (!tetra->S[j])
                    {
                        auto const& opposite = TetrahedronKey<true>::GetOppositeFace();
                        int32_t v0 = tetra->V[opposite[j][0]];
                        int32_t v1 = tetra->V[opposite[j][1]];
                        int32_t v2 = tetra->V[opposite[j][2]];
                        boundary.insert(TriangleKey<true>(v0, v1, v2));
                    }
                }
            }
            return true;
        }

        bool Update(int32_t i)
        {
            // The return value of mGraph.Insert(...) is nullptr if there was
            // a failure to insert.  The Update function will return 'false'
            // when the insertion fails.

            auto const& smap = mGraph.GetTetrahedra();
            Tetrahedron* tetra = smap.begin()->second.get();
            if (GetContainingTetrahedron(i, tetra))
            {
                // The point is inside the convex hull.  The insertion
                // polyhedron contains only tetrahedra in the current
                // tetrahedralization; the hull does not change.

                // Use a depth-first search for those tetrahedra whose
                // circumspheres contain point i.
                std::set<Tetrahedron*> candidates;
                candidates.insert(tetra);

                // Get the boundary of the insertion polyhedron C that
                // contains the tetrahedra whose circumspheres contain point
                // i.  Polyhedron C contains the point i.
                std::set<TriangleKey<true>> boundary;
                if (!GetAndRemoveInsertionPolyhedron(i, candidates, boundary))
                {
                    return false;
                }

                // The insertion polyhedron consists of the tetrahedra formed
                // by point i and the faces of C.
                for (auto const& key : boundary)
                {
                    int32_t v0 = key.V[0];
                    int32_t v1 = key.V[1];
                    int32_t v2 = key.V[2];
                    if (mQuery.ToPlane(i, v0, v1, v2) < 0)
                    {
                        if (!mGraph.Insert(i, v0, v1, v2))
                        {
                            return false;
                        }
                    }
                    // else:  Point i is on an edge or face of 'tetra', so the
                    // subdivision has degenerate tetrahedra.  Ignore these.
                }
            }
            else
            {
                // The point is outside the convex hull.  The insertion
                // polyhedron is formed by point i and any tetrahedra in the
                // current tetrahedralization whose circumspheres contain
                // point i.

                // Locate the convex hull of the tetrahedra.  TODO:  Maintain
                // a hull data structure that is updated incrementally.
                std::set<TriangleKey<true>> hull;
                for (auto const& element : smap)
                {
                    Tetrahedron* t = element.second.get();
                    for (int32_t j = 0; j < 4; ++j)
                    {
                        if (!t->S[j])
                        {
                            auto const& opposite = TetrahedronKey<true>::GetOppositeFace();
                            int32_t v0 = t->V[opposite[j][0]];
                            int32_t v1 = t->V[opposite[j][1]];
                            int32_t v2 = t->V[opposite[j][2]];
                            hull.insert(TriangleKey<true>(v0, v1, v2));
                        }
                    }
                }

                // Iterate over all the hull faces and use the ones visible to
                // point i to locate the insertion polyhedron.
                auto const& tmap = mGraph.GetTriangles();
                std::set<Tetrahedron*> candidates;
                std::set<TriangleKey<true>> visible;
                for (auto const& key : hull)
                {
                    int32_t v0 = key.V[0];
                    int32_t v1 = key.V[1];
                    int32_t v2 = key.V[2];
                    if (mQuery.ToPlane(i, v0, v1, v2) > 0)
                    {
                        auto iter = tmap.find(TriangleKey<false>(v0, v1, v2));
                        if (iter != tmap.end() && iter->second->S[1] == nullptr)
                        {
                            auto adj = iter->second->S[0];
                            if (adj && candidates.find(adj) == candidates.end())
                            {
                                int32_t a0 = adj->V[0];
                                int32_t a1 = adj->V[1];
                                int32_t a2 = adj->V[2];
                                int32_t a3 = adj->V[3];
                                if (mQuery.ToCircumsphere(i, a0, a1, a2, a3) <= 0)
                                {
                                    // Point i is in the circumsphere.
                                    candidates.insert(adj);
                                }
                                else
                                {
                                    // Point i is not in the circumsphere but
                                    // the hull face is visible.
                                    visible.insert(key);
                                }
                            }
                        }
                        else
                        {
                            // TODO:  Add a preprocessor symbol to this file
                            // to allow throwing an exception.  Currently, the
                            // code exits gracefully when floating-point
                            // rounding causes problems.
                            //
                            // LogError("Unexpected condition (ComputeType not exact?)");
                            return false;
                        }
                    }
                }

                // Get the boundary of the insertion subpolyhedron C that
                // contains the tetrahedra whose circumspheres contain
                // point i.
                std::set<TriangleKey<true>> boundary;
                if (!GetAndRemoveInsertionPolyhedron(i, candidates, boundary))
                {
                    return false;
                }

                // The insertion polyhedron P consists of the tetrahedra
                // formed by point i and the back faces of C *and* the visible
                // faces of mGraph-C.
                for (auto const& key : boundary)
                {
                    int32_t v0 = key.V[0];
                    int32_t v1 = key.V[1];
                    int32_t v2 = key.V[2];
                    if (mQuery.ToPlane(i, v0, v1, v2) < 0)
                    {
                        // This is a back face of the boundary.
                        if (!mGraph.Insert(i, v0, v1, v2))
                        {
                            return false;
                        }
                    }
                }
                for (auto const& key : visible)
                {
                    if (!mGraph.Insert(i, key.V[0], key.V[2], key.V[1]))
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        // The epsilon value is used for fuzzy determination of intrinsic
        // dimensionality.  If the dimension is 0, 1, or 2, the constructor
        // returns early.  The caller is responsible for retrieving the
        // dimension and taking an alternate path should the dimension be
        // smaller than 3.  If the dimension is 0, the caller may as well
        // treat all vertices[] as a single point, say, vertices[0].  If the
        // dimension is 1, the caller can query for the approximating line
        // and project vertices[] onto it for further processing.  If the
        // dimension is 2, the caller can query for the approximating plane
        // and project vertices[] onto it for further processing.
        InputType mEpsilon;
        int32_t mDimension;
        Line3<InputType> mLine;
        Plane3<InputType> mPlane;

        // The array of vertices used for geometric queries.  If you want to
        // be certain of a correct result, choose ComputeType to be BSNumber.
        std::vector<Vector3<ComputeType>> mComputeVertices;
        PrimalQuery3<ComputeType> mQuery;

        // The graph information.
        int32_t mNumVertices;
        int32_t mNumUniqueVertices;
        int32_t mNumTetrahedra;
        Vector3<InputType> const* mVertices;
        TSManifoldMesh mGraph;
        std::vector<int32_t> mIndices;
        std::vector<int32_t> mAdjacencies;
    };
}

namespace gte
{
    // The input type must be 'float' or 'double'. The user no longer has
    // the responsibility to specify the compute type.

    template <typename T>
    class Delaunay3<T>
    {
    public:
        // The class is a functor to support computing the Delaunay
        // tetrahedralization of multiple data sets using the same class
        // object.
        Delaunay3()
            :
            mNumVertices(0),
            mVertices(nullptr),
            mIRVertices{},
            mGraph(),
            mDuplicates{},
            mNumUniqueVertices(0),
            mDimension(0),
            mLine(Vector3<T>::Zero(), Vector3<T>::Zero()),
            mPlane(Vector3<T>::Zero(), static_cast<T>(0)),
            mNumTetrahedra(0),
            mIndices{},
            mAdjacencies{},
            mQueryPoint(Vector3<T>::Zero()),
            mIRQueryPoint(Vector3<InputRational>::Zero()),
            mCRPool(maxNumCRPool)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "The input type must be float or double.");
        }

        virtual ~Delaunay3() = default;

        // The input is the array of vertices whose Delaunay
        // tetrahedralization is required. The return value is 'true' if and
        // only if the intrinsic dimension of the points is 3. If the
        // intrinsic dimension is 2, the points lie exactly on a plane which
        // is accessible via GetPlane(). If the intrinsic dimension is 1, the
        // points lie exactly on a line which is accessible via GetLine(). If
        // the intrinsic dimension is 0, the points are all the same point.
        bool operator()(std::vector<Vector3<T>> const& vertices)
        {
            return operator()(vertices.size(), vertices.data());
        }

        bool operator()(size_t numVertices, Vector3<T> const* vertices)
        {
            // Initialize values in case they were set by a previous call
            // to operator()(...).
            LogAssert(
                numVertices > 0 && vertices != nullptr,
                "Invalid argument.");

            T const zero = static_cast<T>(0);
            mNumVertices = numVertices;
            mVertices = vertices;
            mIRVertices.clear();
            mGraph.Clear();
            mDuplicates.clear();
            mNumUniqueVertices = 0;
            mDimension = 0;
            mLine = Line3<T>(Vector3<T>::Zero(), Vector3<T>::Zero());
            mPlane = Plane3<T>(Vector3<T>::Zero(), zero);
            mNumTetrahedra = 0;
            mIndices.clear();
            mAdjacencies.clear();
            mQueryPoint = Vector3<T>::Zero();
            mIRQueryPoint = Vector3<InputRational>::Zero();

            // Compute the intrinsic dimension and return early if that
            // dimension is 0, 1 or 2.
            IntrinsicsVector3<T> info(static_cast<int32_t>(mNumVertices), mVertices, zero);
            if (info.dimension == 0)
            {
                // The vertices are the same point.
                mDimension = 0;
                mLine.origin = info.origin;
                return false;
            }

            if (info.dimension == 1)
            {
                // The vertices are collinear.
                mDimension = 1;
                mLine = Line3<T>(info.origin, info.direction[0]);
                return false;
            }

            if (info.dimension == 2)
            {
                // The vertices are coplanar.
                mDimension = 2;
                mPlane = Plane3<T>(UnitCross(info.direction[0], info.direction[1]),
                    info.origin);
                return false;
            }

            // The vertices necessarily will have a tetrahedralization.
            mDimension = 3;

            // Convert the floating-point inputs to rational type.
            mIRVertices.resize(mNumVertices);
            for (size_t i = 0; i < mNumVertices; ++i)
            {
                mIRVertices[i][0] = mVertices[i][0];
                mIRVertices[i][1] = mVertices[i][1];
                mIRVertices[i][2] = mVertices[i][2];
            }

            // Assume initially the vertices are unique. If duplicates are
            // found during the Delaunay update, mDuplicates[] will be
            // modified accordingly.
            mDuplicates.resize(mNumVertices);
            std::iota(mDuplicates.begin(), mDuplicates.end(), 0);

            // Insert the nondegenerate tetrahedron constructed by the call to
            // IntrinsicsVector2{T}. This is necessary for the circumsphere
            // visibility algorithm to work correctly.
            if (!info.extremeCCW)
            {
                std::swap(info.extreme[2], info.extreme[3]);
            }
            auto inserted = mGraph.Insert(info.extreme[0], info.extreme[1],
                info.extreme[2], info.extreme[3]);
            LogAssert(
                inserted != nullptr,
                "The tetrahedron should not be degenerate.");

            // Incrementally update the tetrahedralization. The set of
            // processed points is maintained to eliminate duplicates.
            ProcessedVertexSet processed{};
            for (size_t i = 0; i < 4; ++i)
            {
                int32_t j = info.extreme[i];
                processed.insert(ProcessedVertex(mVertices[j], j));
                mDuplicates[j] = j;
            }
            for (size_t i = 0; i < mNumVertices; ++i)
            {
                ProcessedVertex v(mVertices[i], i);
                auto iter = processed.find(v);
                if (iter == processed.end())
                {
                    Update(i);
                    processed.insert(v);
                }
                else
                {
                    mDuplicates[i] = iter->location;
                }
            }
            mNumUniqueVertices = processed.size();

            // Assign integer values to the tetrahedra for use by the caller
            // and copy the tetrahedra information to compact arrays mIndices
            // and mAdjacencies.
            UpdateIndicesAdjacencies();

            return true;
        }

        // Dimensional information. If GetDimension() returns 1, the points
        // lie on a line P+t*D. You can sort these if you need a polyline
        // output by projecting onto the line each vertex X = P+t*D, where 
        // t = Dot(D,X-P). If GetDimension() returns 2, the points line on a
        // plane P+s*U+t*V. You can project each vertex X = P+s*U+t*V, where
        // s = Dot(U,X-P) and t = Dot(V,X-P) and then apply Delaunay2 to the
        // (s,t) tuples.
        inline size_t GetDimension() const
        {
            return mDimension;
        }

        inline Line3<T> const& GetLine() const
        {
            return mLine;
        }

        inline Plane3<T> const& GetPlane() const
        {
            return mPlane;
        }

        // Member access.
        inline size_t GetNumVertices() const
        {
            return mIRVertices.size();
        }

        inline Vector3<T> const* GetVertices() const
        {
            return mVertices;
        }

        inline size_t GetNumUniqueVertices() const
        {
            return mNumUniqueVertices;
        }

        // If 'vertices' has no duplicates, GetDuplicates()[i] = i for all i.
        // If vertices[i] is the first occurrence of a vertex and if
        // vertices[j] is found later, then GetDuplicates()[j] = i.
        inline std::vector<size_t> const& GetDuplicates() const
        {
            return mDuplicates;
        }

        inline size_t GetNumTetrahedra() const
        {
            return mNumTetrahedra;
        }

        inline TSManifoldMesh const& GetGraph() const
        {
            return mGraph;
        }

        inline std::vector<int32_t> const& GetIndices() const
        {
            return mIndices;
        }

        inline std::vector<int32_t> const& GetAdjacencies() const
        {
            return mAdjacencies;
        }

        // Locate those tetrahedra faces that do not share other tetrahedra.
        // The returned array has hull.size() = 3*numFaces indices, each
        // triple representing a triangle. The triangles are counterclockwise
        // ordered when viewed from outside the hull. The return value is
        // 'true' iff the dimension is 3.
        bool GetHull(std::vector<size_t>& hull) const
        {
            if (mDimension == 3)
            {
                // Count the number of triangles that are not shared by two
                // tetrahedra.
                size_t numTriangles = 0;
                for (auto adj : mAdjacencies)
                {
                    if (adj == -1)
                    {
                        ++numTriangles;
                    }
                }

                if (numTriangles > 0)
                {
                    // Enumerate the triangles. The prototypical case is the
                    // single tetrahedron V[0] = (0,0,0), V[1] = (1,0,0),
                    // V[2] = (0,1,0) and V[3] = (0,0,1) with no adjacent
                    // tetrahedra. The mIndices[] array is <0,1,2,3>.
                    //   i = 0, face = 0:
                    //    skip index 0, <x,1,2,3>, no swap, triangle = <1,2,3>
                    //   i = 1, face = 1:
                    //    skip index 1, <0,x,2,3>, swap,    triangle = <0,3,2>
                    //   i = 2, face = 2:
                    //    skip index 2, <0,1,x,3>, no swap, triangle = <0,1,3>
                    //   i = 3, face = 3:
                    //    skip index 3, <0,1,2,x>, swap,    triangle = <0,2,1>
                    // To guarantee counterclockwise order of triangles when
                    // viewed outside the tetrahedron, the swap of the last
                    // two indices occurs when face is an odd number;
                    // (face % 2) != 0.
                    hull.resize(3 * numTriangles);
                    size_t current = 0, i = 0;
                    for (auto adj : mAdjacencies)
                    {
                        if (adj == -1)
                        {
                            size_t tetra = i / 4, face = i % 4;
                            for (size_t j = 0; j < 4; ++j)
                            {
                                if (j != face)
                                {
                                    hull[current++] = mIndices[4 * tetra + j];
                                }
                            }
                            if ((face % 2) != 0)
                            {
                                std::swap(hull[current - 1], hull[current - 2]);
                            }
                        }
                        ++i;
                    }
                    return true;
                }
                else
                {
                    LogError("Unexpected. There must be at least one tetrahedron.");
                }
            }
            else
            {
                LogError("The dimension must be 3.");
            }
        }

        // Copy Delaunay tetrahedra to compact arrays mIndices and
        // mAdjacencies. The array information is accessible via the
        // functions GetIndices(size_t, std::array<int32_t, 4>&) and
        // GetAdjacencies(size_t, std::array<int32_t, 4>&).
        void UpdateIndicesAdjacencies()
        {
            // Assign integer values to the tetrahedra for use by the caller.
            auto const& smap = mGraph.GetTetrahedra();
            std::map<Tetrahedron*, int32_t> permute{};
            int32_t i = -1;
            permute[nullptr] = i++;
            for (auto const& element : smap)
            {
                permute[element.second.get()] = i++;
            }

            // Put Delaunay tetrahedra into an array (vertices and adjacency
            // info).
            mNumTetrahedra = smap.size();
            size_t numIndices = 4 * mNumTetrahedra;
            if (mNumTetrahedra > 0)
            {
                mIndices.resize(numIndices);
                mAdjacencies.resize(numIndices);
                i = 0;
                for (auto const& element : smap)
                {
                    Tetrahedron* tetra = element.second.get();
                    for (size_t j = 0; j < 4; ++j, ++i)
                    {
                        mIndices[i] = tetra->V[j];
                        mAdjacencies[i] = permute[tetra->S[j]];
                    }
                }
            }
        }

        // Get the vertex indices for tetrahedron i. The function returns
        // 'true' when the dimension is 3 and i is a valid tetrahedron index,
        // in which case the vertices are valid; otherwise, the function
        // returns 'false' and the vertices are invalid.
        bool GetIndices(size_t t, std::array<int32_t, 4>& indices) const
        {
            if (mDimension == 3)
            {
                size_t const numTetrahedra = mIndices.size() / 4;
                if (t < numTetrahedra)
                {
                    size_t fourI = 4 * static_cast<size_t>(t);
                    indices[0] = mIndices[fourI];
                    indices[1] = mIndices[fourI + 1];
                    indices[2] = mIndices[fourI + 2];
                    indices[3] = mIndices[fourI + 3];
                    return true;
                }
            }
            return false;
        }

        // Get the indices for tetrahedra adjacent to tetrahedron i. The
        // function returns 'true' when the dimension is 3 and i is a valid
        // tetrahedron index, in which case the adjacencies are valid;
        // otherwise, the function returns 'false' and the adjacencies are
        // invalid.
        bool GetAdjacencies(size_t t, std::array<int32_t, 4>& adjacencies) const
        {
            if (mDimension == 3)
            {
                size_t const numTetrahedra = mIndices.size() / 4;
                if (t < numTetrahedra)
                {
                    size_t fourI = 4 * static_cast<size_t>(t);
                    adjacencies[0] = mAdjacencies[fourI];
                    adjacencies[1] = mAdjacencies[fourI + 1];
                    adjacencies[2] = mAdjacencies[fourI + 2];
                    adjacencies[3] = mAdjacencies[fourI + 3];
                    return true;
                }
            }
            return false;
        }

        // Support for searching the tetrahedralization for a tetrahedron
        // that contains a point. If there is a containing tetrahedron, the
        // returned value is a tetrahedron index i with
        // 0 <= t < GetNumTetrahedra(). If there is not a containing
        // tetrahedron, -1 is returned. The computations are performed using
        // exact rational arithmetic.
        //
        // The SearchInfo input stores information about the tetrahedron
        // search when looking for the tetrahedron (if any) that contains p.
        // The first tetrahedron searched is 'initialTetrahedron'. On return
        // 'path' stores those (ordered) tetrahedron indices visited during
        // the search. The last visited tetrahedron has index
        // 'finalTetrahedron' and vertex indices 'finalV[0,1,2,3]', stored in
        // volumetric counterclockwise order. The last face of the search is
        // <finalV[0],finalV[1],finalV[2]>. For spatially coherent inputs p
        // for numerous calls to this function, you will want to specify
        // 'finalTetrahedron' from the previous call as 'initialTetrahedron'
        // for the next call, which should reduce search times.

        static size_t constexpr negOne = std::numeric_limits<size_t>::max();

        struct SearchInfo
        {
            SearchInfo()
                :
                initialTetrahedron(0),
                numPath(0),
                finalTetrahedron(0),
                finalV{ 0, 0, 0, 0 },
                path{}
            {
            }

            size_t initialTetrahedron;
            size_t numPath;
            size_t finalTetrahedron;
            std::array<int32_t, 4> finalV;
            std::vector<size_t> path;
        };

        // If the point is in a tetrahedron, the return value is the index of
        // the tetrahedron. If the point is not in a tetrahedron, the return
        // value isstd::numeric_limits<size_t>::max().
        size_t GetContainingTetrahedron(Vector3<T> const& inP, SearchInfo& info) const
        {
            LogAssert(
                mDimension == 3,
                "Invalid dimension for tetrahedron search.");

            mQueryPoint = inP;
            mIRQueryPoint = { inP[0], inP[1] };

            size_t const numTetrahedra = mIndices.size() / 4;
            info.path.resize(numTetrahedra);
            info.numPath = 0;
            size_t tetrahedron{};
            if (info.initialTetrahedron < numTetrahedra)
            {
                tetrahedron = info.initialTetrahedron;
            }
            else
            {
                info.initialTetrahedron = 0;
                tetrahedron = 0;
            }

            // Use tetrahedron faces as binary separating planes.
            int32_t adjacent{};
            for (size_t i = 0; i < numTetrahedra; ++i)
            {
                size_t ibase = 4 * tetrahedron;
                int32_t const* v = &mIndices[ibase];

                info.path[info.numPath++] = tetrahedron;
                info.finalTetrahedron = tetrahedron;
                info.finalV[0] = v[0];
                info.finalV[1] = v[1];
                info.finalV[2] = v[2];
                info.finalV[3] = v[3];

                // <V1,V2,V3> counterclockwise when viewed outside
                // tetrahedron.
                if (ToPlane(negOne, v[1], v[2], v[3]) > 0)
                {
                    adjacent = mAdjacencies[ibase];
                    if (adjacent == -1)
                    {
                        info.finalV[0] = v[1];
                        info.finalV[1] = v[2];
                        info.finalV[2] = v[3];
                        info.finalV[3] = v[0];
                        return negOne;
                    }
                    tetrahedron = static_cast<size_t>(adjacent);
                    continue;
                }

                // <V0,V3,V2> counterclockwise when viewed outside
                // tetrahedron.
                if (ToPlane(negOne, v[0], v[2], v[3]) < 0)
                {
                    adjacent = mAdjacencies[static_cast<size_t>(ibase) + 1];
                    if (adjacent == -1)
                    {
                        info.finalV[0] = v[0];
                        info.finalV[1] = v[2];
                        info.finalV[2] = v[3];
                        info.finalV[3] = v[1];
                        return negOne;
                    }
                    tetrahedron = static_cast<size_t>(adjacent);
                    continue;
                }

                // <V0,V1,V3> counterclockwise when viewed outside
                // tetrahedron.
                if (ToPlane(negOne, v[0], v[1], v[3]) > 0)
                {
                    adjacent = mAdjacencies[static_cast<size_t>(ibase) + 2];
                    if (adjacent == -1)
                    {
                        info.finalV[0] = v[0];
                        info.finalV[1] = v[1];
                        info.finalV[2] = v[3];
                        info.finalV[3] = v[2];
                        return negOne;
                    }
                    tetrahedron = static_cast<size_t>(adjacent);
                    continue;
                }

                // <V0,V2,V1> counterclockwise when viewed outside
                // tetrahedron.
                if (ToPlane(negOne, v[0], v[1], v[2]) < 0)
                {
                    adjacent = mAdjacencies[static_cast<size_t>(ibase) + 3];
                    if (adjacent == -1)
                    {
                        info.finalV[0] = v[0];
                        info.finalV[1] = v[1];
                        info.finalV[2] = v[2];
                        info.finalV[3] = v[3];
                        return negOne;
                    }
                    tetrahedron = static_cast<size_t>(adjacent);
                    continue;
                }

                return tetrahedron;
            }

            LogError(
                "Unexpected termination of loop while searching for a triangle.");
        }

    protected:
        // The type of the read-only input vertices[] when converted for
        // rational arithmetic.
        static int32_t constexpr InputNumWords = std::is_same<T, float>::value ? 2 : 4;
        using InputRational = BSNumber<UIntegerFP32<InputNumWords>>;

        // The vector of vertices used for geometric queries. The input
        // vertices are read-only, so we can represent them by the type
        // InputRational.
        size_t mNumVertices;
        Vector3<T> const* mVertices;
        std::vector<Vector3<InputRational>> mIRVertices;

        TSManifoldMesh mGraph;

    private:
        // The compute type used for exact sign classification.
        static int32_t constexpr ComputeNumWords = std::is_same<T, float>::value ? 44 : 330;
        using ComputeRational = BSNumber<UIntegerFP32<ComputeNumWords>>;

        // Convenient renaming.
        typedef TSManifoldMesh::Tetrahedron Tetrahedron;

        struct ProcessedVertex
        {
            ProcessedVertex()
                :
                vertex(Vector3<T>::Zero()),
                location(0)
            {
            }

            ProcessedVertex(Vector3<T> const& inVertex, size_t inLocation)
                :
                vertex(inVertex),
                location(inLocation)
            {
            }

            // Support for hashing in std::unordered_set<>. The first
            // operator() is the hash function. The second operator() is
            // the equality comparison used for elements in the same bucket.
            size_t operator()(ProcessedVertex const& v) const
            {
                return HashValue(v.vertex[0], v.vertex[1], v.vertex[2], v.location);
            }

            bool operator()(ProcessedVertex const& v0, ProcessedVertex const& v1) const
            {
                return v0.vertex == v1.vertex && v0.location == v1.location;
            }

            Vector3<T> vertex;
            size_t location;
        };

        using ProcessedVertexSet = std::unordered_set<
            ProcessedVertex, ProcessedVertex, ProcessedVertex>;

        using DirectedTriangleKeySet = std::unordered_set<
            TriangleKey<true>, TriangleKey<true>, TriangleKey<true>>;

        using TetrahedronPtrSet = std::unordered_set<Tetrahedron*>;

        static ComputeRational const& Copy(InputRational const& source,
            ComputeRational& target)
        {
            target.SetSign(source.GetSign());
            target.SetBiasedExponent(source.GetBiasedExponent());
            target.GetUInteger().CopyFrom(source.GetUInteger());
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            target.mValue = (double)target;
#endif
            return target;
        }

        // Given a plane with origin V0 and normal N = Cross(V1-V0,V2-V0)
        // and given a query point P, ToPlane returns
        //   +1, P on positive side of plane (side to which N points)
        //   -1, P on negative side of plane (side to which -N points)
        //    0, P on the plane
        int32_t ToPlane(size_t pIndex, size_t v0Index, size_t v1Index, size_t v2Index) const
        {
            // The expression tree has 34 nodes consisting of 12 input
            // leaves and 22 compute nodes.

            // Use interval arithmetic to determine the sign if possible.
            auto const& inP = (pIndex != negOne ? mVertices[pIndex] : mQueryPoint);
            Vector3<T> const& inV0 = mVertices[v0Index];
            Vector3<T> const& inV1 = mVertices[v1Index];
            Vector3<T> const& inV2 = mVertices[v2Index];

            // Evaluate the expression tree of intervals.
            auto x0 = SWInterval<T>::Sub(inP[0], inV0[0]);
            auto y0 = SWInterval<T>::Sub(inP[1], inV0[1]);
            auto z0 = SWInterval<T>::Sub(inP[2], inV0[2]);
            auto x1 = SWInterval<T>::Sub(inV1[0], inV0[0]);
            auto y1 = SWInterval<T>::Sub(inV1[1], inV0[1]);
            auto z1 = SWInterval<T>::Sub(inV1[2], inV0[2]);
            auto x2 = SWInterval<T>::Sub(inV2[0], inV0[0]);
            auto y2 = SWInterval<T>::Sub(inV2[1], inV0[1]);
            auto z2 = SWInterval<T>::Sub(inV2[2], inV0[2]);
            auto y0z1 = y0 * z1;
            auto y0z2 = y0 * z2;
            auto y1z0 = y1 * z0;
            auto y1z2 = y1 * z2;
            auto y2z0 = y2 * z0;
            auto y2z1 = y2 * z1;
            auto c0 = y1z2 - y2z1;
            auto c1 = y2z0 - y0z2;
            auto c2 = y0z1 - y1z0;
            auto x0c0 = x0 * c0;
            auto x1c1 = x1 * c1;
            auto x2c2 = x2 * c2;
            auto det = x0c0 + x1c1 + x2c2;

            T constexpr zero = 0;
            if (det[0] > zero)
            {
                return +1;
            }
            else if (det[1] < zero)
            {
                return -1;
            }

            // The exact sign of the determinant is not known, so compute
            // the determinant using rational arithmetic.

            // Name the nodes of the expression tree.
            auto const& irP = (pIndex != negOne ? mIRVertices[pIndex] : mIRQueryPoint);
            Vector3<InputRational> const& irV0 = mIRVertices[v0Index];
            Vector3<InputRational> const& irV1 = mIRVertices[v1Index];
            Vector3<InputRational> const& irV2 = mIRVertices[v2Index];

            // Input nodes.
            auto const& crP0 = Copy(irP[0], mCRPool[0]);
            auto const& crP1 = Copy(irP[1], mCRPool[1]);
            auto const& crP2 = Copy(irP[2], mCRPool[2]);
            auto const& crV00 = Copy(irV0[0], mCRPool[3]);
            auto const& crV01 = Copy(irV0[1], mCRPool[4]);
            auto const& crV02 = Copy(irV0[2], mCRPool[5]);
            auto const& crV10 = Copy(irV1[0], mCRPool[6]);
            auto const& crV11 = Copy(irV1[1], mCRPool[7]);
            auto const& crV12 = Copy(irV1[2], mCRPool[8]);
            auto const& crV20 = Copy(irV2[0], mCRPool[9]);
            auto const& crV21 = Copy(irV2[1], mCRPool[10]);
            auto const& crV22 = Copy(irV2[2], mCRPool[11]);

            // Compute nodes.
            auto& crX0 = mCRPool[12];
            auto& crY0 = mCRPool[13];
            auto& crZ0 = mCRPool[14];
            auto& crX1 = mCRPool[15];
            auto& crY1 = mCRPool[16];
            auto& crZ1 = mCRPool[17];
            auto& crX2 = mCRPool[18];
            auto& crY2 = mCRPool[19];
            auto& crZ2 = mCRPool[20];
            auto& crY0Z1 = mCRPool[21];
            auto& crY0Z2 = mCRPool[22];
            auto& crY1Z0 = mCRPool[23];
            auto& crY1Z2 = mCRPool[24];
            auto& crY2Z0 = mCRPool[25];
            auto& crY2Z1 = mCRPool[26];
            auto& crC0 = mCRPool[27];
            auto& crC1 = mCRPool[28];
            auto& crC2 = mCRPool[29];
            auto& crX0C0 = mCRPool[30];
            auto& crX1C1 = mCRPool[31];
            auto& crX2C2 = mCRPool[32];
            auto& crDet = mCRPool[33];

            // Evaluate the expression tree of rational numbers.
            crX0 = crP0 - crV00;
            crY0 = crP1 - crV01;
            crZ0 = crP2 - crV02;
            crX1 = crV10 - crV00;
            crY1 = crV11 - crV01;
            crZ1 = crV12 - crV02;
            crX2 = crV20 - crV00;
            crY2 = crV21 - crV01;
            crZ2 = crV22 - crV02;
            crY0Z1 = crY0 * crZ1;
            crY0Z2 = crY0 * crZ2;
            crY1Z0 = crY1 * crZ0;
            crY1Z2 = crY1 * crZ2;
            crY2Z0 = crY2 * crZ0;
            crY2Z1 = crY2 * crZ1;
            crC0 = crY1Z2 - crY2Z1;
            crC1 = crY2Z0 - crY0Z2;
            crC2 = crY0Z1 - crY1Z0;
            crX0C0 = crX0 * crC0;
            crX1C1 = crX1 * crC1;
            crX2C2 = crX2 * crC2;
            crDet = crX0C0 + crX1C1 + crX2C2;
            return crDet.GetSign();
        }

        // For a tetrahedron with vertices ordered as described in the file
        // TetrahedronKey.h, the function returns
        //   +1, P outside circumsphere of tetrahedron
        //   -1, P inside circumsphere of tetrahedron
        //    0, P on circumsphere of tetrahedron
        int32_t ToCircumsphere(size_t pIndex, size_t v0Index, size_t v1Index,
            size_t v2Index, size_t v3Index) const
        {
            // The expression tree has 98 nodes consisting of 15 input
            // leaves and 83 compute nodes.

            // Use interval arithmetic to determine the sign if possible.
            auto const& inP = (pIndex != negOne ? mVertices[pIndex] : mQueryPoint);
            Vector3<T> const& inV0 = mVertices[v0Index];
            Vector3<T> const& inV1 = mVertices[v1Index];
            Vector3<T> const& inV2 = mVertices[v2Index];
            Vector3<T> const& inV3 = mVertices[v3Index];

            // Evaluate the expression tree of intervals.
            auto x0 = SWInterval<T>::Sub(inV0[0], inP[0]);
            auto y0 = SWInterval<T>::Sub(inV0[1], inP[1]);
            auto z0 = SWInterval<T>::Sub(inV0[2], inP[2]);
            auto s00 = SWInterval<T>::Add(inV0[0], inP[0]);
            auto s01 = SWInterval<T>::Add(inV0[1], inP[1]);
            auto s02 = SWInterval<T>::Add(inV0[2], inP[2]);
            auto x1 = SWInterval<T>::Sub(inV1[0], inP[0]);
            auto y1 = SWInterval<T>::Sub(inV1[1], inP[1]);
            auto z1 = SWInterval<T>::Sub(inV1[2], inP[2]);
            auto s10 = SWInterval<T>::Add(inV1[0], inP[0]);
            auto s11 = SWInterval<T>::Add(inV1[1], inP[1]);
            auto s12 = SWInterval<T>::Add(inV1[2], inP[2]);
            auto x2 = SWInterval<T>::Sub(inV2[0], inP[0]);
            auto y2 = SWInterval<T>::Sub(inV2[1], inP[1]);
            auto z2 = SWInterval<T>::Sub(inV2[2], inP[2]);
            auto s20 = SWInterval<T>::Add(inV2[0], inP[0]);
            auto s21 = SWInterval<T>::Add(inV2[1], inP[1]);
            auto s22 = SWInterval<T>::Add(inV2[2], inP[2]);
            auto x3 = SWInterval<T>::Sub(inV3[0], inP[0]);
            auto y3 = SWInterval<T>::Sub(inV3[1], inP[1]);
            auto z3 = SWInterval<T>::Sub(inV3[2], inP[2]);
            auto s30 = SWInterval<T>::Add(inV3[0], inP[0]);
            auto s31 = SWInterval<T>::Add(inV3[1], inP[1]);
            auto s32 = SWInterval<T>::Add(inV3[2], inP[2]);
            auto t00 = s00 * x0;
            auto t01 = s01 * y0;
            auto t02 = s02 * z0;
            auto t10 = s10 * x1;
            auto t11 = s11 * y1;
            auto t12 = s12 * z1;
            auto t20 = s20 * x2;
            auto t21 = s21 * y2;
            auto t22 = s22 * z2;
            auto t30 = s30 * x3;
            auto t31 = s31 * y3;
            auto t32 = s32 * z3;
            auto w0 = t00 + t01 + t02;
            auto w1 = t10 + t11 + t12;
            auto w2 = t20 + t21 + t22;
            auto w3 = t30 + t31 + t32;
            auto x0y1 = x0 * y1;
            auto x0y2 = x0 * y2;
            auto x0y3 = x0 * y3;
            auto x1y0 = x1 * y0;
            auto x1y2 = x1 * y2;
            auto x1y3 = x1 * y3;
            auto x2y0 = x2 * y0;
            auto x2y1 = x2 * y1;
            auto x2y3 = x2 * y3;
            auto x3y0 = x3 * y0;
            auto x3y1 = x3 * y1;
            auto x3y2 = x3 * y2;
            auto z0w1 = z0 * w1;
            auto z0w2 = z0 * w2;
            auto z0w3 = z0 * w3;
            auto z1w0 = z1 * w0;
            auto z1w2 = z1 * w2;
            auto z1w3 = z1 * w3;
            auto z2w0 = z2 * w0;
            auto z2w1 = z2 * w1;
            auto z2w3 = z2 * w3;
            auto z3w0 = z3 * w0;
            auto z3w1 = z3 * w1;
            auto z3w2 = z3 * w2;
            auto u0 = x0y1 - x1y0;
            auto u1 = x0y2 - x2y0;
            auto u2 = x0y3 - x3y0;
            auto u3 = x1y2 - x2y1;
            auto u4 = x1y3 - x3y1;
            auto u5 = x2y3 - x3y2;
            auto v0 = z0w1 - z1w0;
            auto v1 = z0w2 - z2w0;
            auto v2 = z0w3 - z3w0;
            auto v3 = z1w2 - z2w1;
            auto v4 = z1w3 - z3w1;
            auto v5 = z2w3 - z3w2;
            auto u0v5 = u0 * v5;
            auto u1v4 = u1 * v4;
            auto u2v3 = u2 * v3;
            auto u3v2 = u3 * v2;
            auto u4v1 = u4 * v1;
            auto u5v0 = u5 * v0;
            auto det = u0v5 - u1v4 + u2v3 + u3v2 - u4v1 + u5v0;

            T constexpr zero = 0;
            if (det[0] > zero)
            {
                return +1;
            }
            else if (det[1] < zero)
            {
                return -1;
            }

            // The exact sign of the determinant is not known, so compute
            // the determinant using rational arithmetic.

            // Name the nodes of the expression tree.
            auto const& irP = (pIndex != negOne ? mIRVertices[pIndex] : mIRQueryPoint);
            Vector3<InputRational> const& irV0 = mIRVertices[v0Index];
            Vector3<InputRational> const& irV1 = mIRVertices[v1Index];
            Vector3<InputRational> const& irV2 = mIRVertices[v2Index];
            Vector3<InputRational> const& irV3 = mIRVertices[v3Index];

            // Input nodes.
            auto const& crP0 = Copy(irP[0], mCRPool[0]);
            auto const& crP1 = Copy(irP[1], mCRPool[1]);
            auto const& crP2 = Copy(irP[2], mCRPool[2]);
            auto const& crV00 = Copy(irV0[0], mCRPool[3]);
            auto const& crV01 = Copy(irV0[1], mCRPool[4]);
            auto const& crV02 = Copy(irV0[2], mCRPool[5]);
            auto const& crV10 = Copy(irV1[0], mCRPool[6]);
            auto const& crV11 = Copy(irV1[1], mCRPool[7]);
            auto const& crV12 = Copy(irV1[2], mCRPool[8]);
            auto const& crV20 = Copy(irV2[0], mCRPool[9]);
            auto const& crV21 = Copy(irV2[1], mCRPool[10]);
            auto const& crV22 = Copy(irV2[2], mCRPool[11]);
            auto const& crV30 = Copy(irV3[0], mCRPool[12]);
            auto const& crV31 = Copy(irV3[1], mCRPool[13]);
            auto const& crV32 = Copy(irV3[2], mCRPool[14]);

            // Compute nodes.
            auto& crX0 = mCRPool[15];
            auto& crY0 = mCRPool[16];
            auto& crZ0 = mCRPool[17];
            auto& crS00 = mCRPool[18];
            auto& crS01 = mCRPool[19];
            auto& crS02 = mCRPool[20];
            auto& crX1 = mCRPool[21];
            auto& crY1 = mCRPool[22];
            auto& crZ1 = mCRPool[23];
            auto& crS10 = mCRPool[24];
            auto& crS11 = mCRPool[25];
            auto& crS12 = mCRPool[26];
            auto& crX2 = mCRPool[27];
            auto& crY2 = mCRPool[28];
            auto& crZ2 = mCRPool[29];
            auto& crS20 = mCRPool[30];
            auto& crS21 = mCRPool[31];
            auto& crS22 = mCRPool[32];
            auto& crX3 = mCRPool[33];
            auto& crY3 = mCRPool[34];
            auto& crZ3 = mCRPool[35];
            auto& crS30 = mCRPool[36];
            auto& crS31 = mCRPool[37];
            auto& crS32 = mCRPool[38];
            auto& crT00 = mCRPool[39];
            auto& crT01 = mCRPool[40];
            auto& crT02 = mCRPool[41];
            auto& crT10 = mCRPool[42];
            auto& crT11 = mCRPool[43];
            auto& crT12 = mCRPool[44];
            auto& crT20 = mCRPool[45];
            auto& crT21 = mCRPool[46];
            auto& crT22 = mCRPool[47];
            auto& crT30 = mCRPool[48];
            auto& crT31 = mCRPool[49];
            auto& crT32 = mCRPool[50];
            auto& crW0 = mCRPool[51];
            auto& crW1 = mCRPool[52];
            auto& crW2 = mCRPool[53];
            auto& crW3 = mCRPool[54];
            auto& crX0Y1 = mCRPool[55];
            auto& crX0Y2 = mCRPool[56];
            auto& crX0Y3 = mCRPool[57];
            auto& crX1Y0 = mCRPool[58];
            auto& crX1Y2 = mCRPool[59];
            auto& crX1Y3 = mCRPool[60];
            auto& crX2Y0 = mCRPool[61];
            auto& crX2Y1 = mCRPool[62];
            auto& crX2Y3 = mCRPool[63];
            auto& crX3Y0 = mCRPool[64];
            auto& crX3Y1 = mCRPool[65];
            auto& crX3Y2 = mCRPool[66];
            auto& crZ0W1 = mCRPool[67];
            auto& crZ0W2 = mCRPool[68];
            auto& crZ0W3 = mCRPool[69];
            auto& crZ1W0 = mCRPool[70];
            auto& crZ1W2 = mCRPool[71];
            auto& crZ1W3 = mCRPool[72];
            auto& crZ2W0 = mCRPool[73];
            auto& crZ2W1 = mCRPool[74];
            auto& crZ2W3 = mCRPool[75];
            auto& crZ3W0 = mCRPool[76];
            auto& crZ3W1 = mCRPool[77];
            auto& crZ3W2 = mCRPool[78];
            auto& crU0 = mCRPool[79];
            auto& crU1 = mCRPool[80];
            auto& crU2 = mCRPool[81];
            auto& crU3 = mCRPool[82];
            auto& crU4 = mCRPool[83];
            auto& crU5 = mCRPool[84];
            auto& crV0 = mCRPool[85];
            auto& crV1 = mCRPool[86];
            auto& crV2 = mCRPool[87];
            auto& crV3 = mCRPool[88];
            auto& crV4 = mCRPool[89];
            auto& crV5 = mCRPool[90];
            auto& crU0V5 = mCRPool[91];
            auto& crU1V4 = mCRPool[92];
            auto& crU2V3 = mCRPool[93];
            auto& crU3V2 = mCRPool[94];
            auto& crU4V1 = mCRPool[95];
            auto& crU5V0 = mCRPool[96];
            auto& crDet = mCRPool[97];

            // Evaluate the expression tree of rational numbers.
            crX0 = crV00 - crP0;
            crY0 = crV01 - crP1;
            crZ0 = crV02 - crP2;
            crS00 = crV00 + crP0;
            crS01 = crV01 + crP1;
            crS02 = crV02 + crP2;
            crX1 = crV10 - crP0;
            crY1 = crV11 - crP1;
            crZ1 = crV12 - crP2;
            crS10 = crV10 + crP0;
            crS11 = crV11 + crP1;
            crS12 = crV12 + crP2;
            crX2 = crV20 - crP0;
            crY2 = crV21 - crP1;
            crZ2 = crV22 - crP2;
            crS20 = crV20 + crP0;
            crS21 = crV21 + crP1;
            crS22 = crV22 + crP2;
            crX3 = crV30 - crP0;
            crY3 = crV31 - crP1;
            crZ3 = crV32 - crP2;
            crS30 = crV30 + crP0;
            crS31 = crV31 + crP1;
            crS32 = crV32 + crP2;
            crT00 = crS00 * crX0;
            crT01 = crS01 * crY0;
            crT02 = crS02 * crZ0;
            crT10 = crS10 * crX1;
            crT11 = crS11 * crY1;
            crT12 = crS12 * crZ1;
            crT20 = crS20 * crX2;
            crT21 = crS21 * crY2;
            crT22 = crS22 * crZ2;
            crT30 = crS30 * crX3;
            crT31 = crS31 * crY3;
            crT32 = crS32 * crZ3;
            crW0 = crT00 + crT01 + crT02;
            crW1 = crT10 + crT11 + crT12;
            crW2 = crT20 + crT21 + crT22;
            crW3 = crT30 + crT31 + crT32;
            crX0Y1 = crX0 * crY1;
            crX0Y2 = crX0 * crY2;
            crX0Y3 = crX0 * crY3;
            crX1Y0 = crX1 * crY0;
            crX1Y2 = crX1 * crY2;
            crX1Y3 = crX1 * crY3;
            crX2Y0 = crX2 * crY0;
            crX2Y1 = crX2 * crY1;
            crX2Y3 = crX2 * crY3;
            crX3Y0 = crX3 * crY0;
            crX3Y1 = crX3 * crY1;
            crX3Y2 = crX3 * crY2;
            crZ0W1 = crZ0 * crW1;
            crZ0W2 = crZ0 * crW2;
            crZ0W3 = crZ0 * crW3;
            crZ1W0 = crZ1 * crW0;
            crZ1W2 = crZ1 * crW2;
            crZ1W3 = crZ1 * crW3;
            crZ2W0 = crZ2 * crW0;
            crZ2W1 = crZ2 * crW1;
            crZ2W3 = crZ2 * crW3;
            crZ3W0 = crZ3 * crW0;
            crZ3W1 = crZ3 * crW1;
            crZ3W2 = crZ3 * crW2;
            crU0 = crX0Y1 - crX1Y0;
            crU1 = crX0Y2 - crX2Y0;
            crU2 = crX0Y3 - crX3Y0;
            crU3 = crX1Y2 - crX2Y1;
            crU4 = crX1Y3 - crX3Y1;
            crU5 = crX2Y3 - crX3Y2;
            crV0 = crZ0W1 - crZ1W0;
            crV1 = crZ0W2 - crZ2W0;
            crV2 = crZ0W3 - crZ3W0;
            crV3 = crZ1W2 - crZ2W1;
            crV4 = crZ1W3 - crZ3W1;
            crV5 = crZ2W3 - crZ3W2;
            crU0V5 = crU0 * crV5;
            crU1V4 = crU1 * crV4;
            crU2V3 = crU2 * crV3;
            crU3V2 = crU3 * crV2;
            crU4V1 = crU4 * crV1;
            crU5V0 = crU5 * crV0;
            crDet = crU0V5 - crU1V4 + crU2V3 + crU3V2 - crU4V1 + crU5V0;
            return crDet.GetSign();
        }

        bool GetContainingTetrahedron(size_t pIndex, Tetrahedron*& tetra) const
        {
            size_t const numTetrahedra = mGraph.GetTetrahedra().size();
            for (size_t t = 0; t < numTetrahedra; ++t)
            {
                size_t j;
                for (j = 0; j < 4; ++j)
                {
                    auto const& opposite = TetrahedronKey<true>::GetOppositeFace();
                    size_t v0Index = tetra->V[opposite[j][0]];
                    size_t v1Index = tetra->V[opposite[j][1]];
                    size_t v2Index = tetra->V[opposite[j][2]];
                    if (ToPlane(pIndex, v0Index, v1Index, v2Index) > 0)
                    {
                        // Point i sees face <v0,v1,v2> from outside the
                        // tetrahedron.
                        auto adjTetra = tetra->S[j];
                        if (adjTetra)
                        {
                            // Traverse to the tetrahedron sharing the face.
                            tetra = adjTetra;
                            break;
                        }
                        else
                        {
                            // We reached a hull face, so the point is outside
                            // the hull.
                            return false;
                        }
                    }

                }

                if (j == 4)
                {
                    // The point is inside all four faces, so the point is inside
                    // a tetrahedron.
                    return true;
                }
            }

            LogError(
                "Unexpected termination of loop.");
        }

        void GetAndRemoveInsertionPolyhedron(size_t pIndex,
            TetrahedronPtrSet& candidates, DirectedTriangleKeySet& boundary)
        {
            // Locate the tetrahedra that make up the insertion polyhedron.
            TSManifoldMesh polyhedron;
            while (candidates.size() > 0)
            {
                Tetrahedron* tetra = *candidates.begin();
                candidates.erase(candidates.begin());

                for (size_t j = 0; j < 4; ++j)
                {
                    auto adj = tetra->S[j];
                    if (adj && candidates.find(adj) == candidates.end())
                    {
                        size_t v0Index = adj->V[0];
                        size_t v1Index = adj->V[1];
                        size_t v2Index = adj->V[2];
                        size_t v3Index = adj->V[3];
                        if (ToCircumsphere(pIndex, v0Index, v1Index, v2Index, v3Index) <= 0)
                        {
                            // Point P is in the circumsphere.
                            candidates.insert(adj);
                        }
                    }
                }

                auto inserted = polyhedron.Insert(tetra->V[0], tetra->V[1],
                    tetra->V[2], tetra->V[3]);
                LogAssert(
                    inserted != nullptr,
                    "Unexpected insertion failure.");

                auto removed = mGraph.Remove(tetra->V[0], tetra->V[1],
                    tetra->V[2], tetra->V[3]);
                LogAssert(
                    removed,
                    "Unexpected removal failure.");
            }

            // Get the boundary triangles of the insertion polyhedron.
            for (auto const& element : polyhedron.GetTetrahedra())
            {
                Tetrahedron* tetra = element.second.get();
                for (size_t j = 0; j < 4; ++j)
                {
                    if (!tetra->S[j])
                    {
                        auto const& opposite = TetrahedronKey<true>::GetOppositeFace();
                        int32_t v0 = tetra->V[opposite[j][0]];
                        int32_t v1 = tetra->V[opposite[j][1]];
                        int32_t v2 = tetra->V[opposite[j][2]];
                        boundary.insert(TriangleKey<true>(v0, v1, v2));
                    }
                }
            }
        }

        void Update(size_t pIndex)
        {
            auto const& smap = mGraph.GetTetrahedra();
            Tetrahedron* tetra = smap.begin()->second.get();
            if (GetContainingTetrahedron(pIndex, tetra))
            {
                // The point is inside the convex hull. The insertion
                // polyhedron contains only tetrahedra in the current
                // tetrahedralization; the hull does not change.

                // Use a depth-first search for those tetrahedra whose
                // circumspheres contain point P.
                TetrahedronPtrSet candidates{};
                candidates.insert(tetra);

                // Get the boundary of the insertion polyhedron C that
                // contains the tetrahedra whose circumspheres contain point
                // P. Polyhedron C contains this point.
                DirectedTriangleKeySet boundary{};
                GetAndRemoveInsertionPolyhedron(pIndex, candidates, boundary);

                // The insertion polyhedron consists of the tetrahedra formed
                // by point i and the faces of C.
                for (auto const& key : boundary)
                {
                    size_t v0Index = static_cast<size_t>(key.V[0]);
                    size_t v1Index = static_cast<size_t>(key.V[1]);
                    size_t v2Index = static_cast<size_t>(key.V[2]);
                    if (ToPlane(pIndex, v0Index, v1Index, v2Index) < 0)
                    {
                        auto inserted = mGraph.Insert(static_cast<int32_t>(pIndex),
                            key.V[0], key.V[1], key.V[2]);
                        LogAssert(
                            inserted != nullptr,
                            "Unexpected insertion failure.");
                    }
                }
            }
            else
            {
                // The point is outside the convex hull. The insertion
                // polyhedron is formed by point P and any tetrahedra in the
                // current tetrahedralization whose circumspheres contain
                // point P.

                // Locate the convex hull of the tetrahedra.
                DirectedTriangleKeySet hull{};
                for (auto const& element : smap)
                {
                    Tetrahedron* t = element.second.get();
                    for (size_t j = 0; j < 4; ++j)
                    {
                        if (!t->S[j])
                        {
                            auto const& opposite = TetrahedronKey<true>::GetOppositeFace();
                            hull.insert(TriangleKey<true>(t->V[opposite[j][0]],
                                t->V[opposite[j][1]], t->V[opposite[j][2]]));
                        }
                    }
                }

                // Iterate over all the hull faces and use the ones visible to
                // point i to locate the insertion polyhedron.
                auto const& tmap = mGraph.GetTriangles();
                TetrahedronPtrSet candidates{};
                DirectedTriangleKeySet visible{};
                for (auto const& key : hull)
                {
                    size_t v0Index = static_cast<size_t>(key.V[0]);
                    size_t v1Index = static_cast<size_t>(key.V[1]);
                    size_t v2Index = static_cast<size_t>(key.V[2]);
                    if (ToPlane(pIndex, v0Index, v1Index, v2Index) > 0)
                    {
                        auto iter = tmap.find(TriangleKey<false>(key.V[0], key.V[1], key.V[2]));
                        if (iter != tmap.end() && iter->second->S[1] == nullptr)
                        {
                            auto adj = iter->second->S[0];
                            if (adj && candidates.find(adj) == candidates.end())
                            {
                                size_t a0Index = static_cast<size_t>(adj->V[0]);
                                size_t a1Index = static_cast<size_t>(adj->V[1]);
                                size_t a2Index = static_cast<size_t>(adj->V[2]);
                                size_t a3Index = static_cast<size_t>(adj->V[3]);
                                if (ToCircumsphere(pIndex, a0Index, a1Index, a2Index,
                                    a3Index) <= 0)
                                {
                                    // Point P is in the circumsphere.
                                    candidates.insert(adj);
                                }
                                else
                                {
                                    // Point P is not in the circumsphere but
                                    // the hull face is visible.
                                    visible.insert(key);
                                }
                            }
                        }
                        else
                        {
                            LogError(
                                "This condition should not occur for rational arithmetic.");
                        }
                    }
                }

                // Get the boundary of the insertion subpolyhedron C that
                // contains the tetrahedra whose circumspheres contain
                // point P.
                DirectedTriangleKeySet boundary{};
                GetAndRemoveInsertionPolyhedron(pIndex, candidates, boundary);

                // The insertion polyhedron P consists of the tetrahedra
                // formed by point i and the back faces of C *and* the visible
                // faces of mGraph-C.
                for (auto const& key : boundary)
                {
                    size_t v0Index = static_cast<size_t>(key.V[0]);
                    size_t v1Index = static_cast<size_t>(key.V[1]);
                    size_t v2Index = static_cast<size_t>(key.V[2]);
                    if (ToPlane(pIndex, v0Index, v1Index, v2Index) < 0)
                    {
                        // This is a back face of the boundary.
                        auto inserted = mGraph.Insert(static_cast<int32_t>(pIndex),
                            key.V[0], key.V[1], key.V[2]);
                        LogAssert(
                            inserted != nullptr,
                            "Unexpected insertion failure.");
                    }
                }
                for (auto const& key : visible)
                {
                    auto inserted = mGraph.Insert(static_cast<int32_t>(pIndex),
                        key.V[0], key.V[2], key.V[1]);
                    LogAssert(
                        inserted != nullptr,
                        "Unexpected insertion failure.");
                }
            }
        }

        // If a vertex occurs multiple times in the 'vertices' input to the
        // constructor, the first processed occurrence of that vertex has an
        // index stored in this array. If there are no duplicates, then
        // mDuplicates[i] = i for all i.
        std::vector<size_t> mDuplicates;
        size_t mNumUniqueVertices;

        // If the intrinsic dimension of the input vertices is 0, 1 or 2, the
        // constructor returns early. The caller is responsible for retrieving
        // the dimension and taking an alternate path should the dimension be
        // smaller than 3. If the dimension is 0, all vertices are the same.
        // If the dimension is 1, the vertices lie on a line, in which case
        // the caller can project vertices[] onto the line for further
        // processing. If the dimension is 2, the vertices lie on a plane, in
        // which case the caller can project vertices[] onto the plane for
        // further processing.
        size_t mDimension;
        Line3<T> mLine;
        Plane3<T> mPlane;

        // These are computed by UpdateIndicesAdjacencies(). They are used
        // for point-containment queries in the tetrahedron mesh.
        size_t mNumTetrahedra;
        std::vector<int32_t> mIndices;
        std::vector<int32_t> mAdjacencies;

    private:
        // The query point for Update, GetContainingTriangle and
        // GetAndRemoveInsertionPolyhedron when the point is not an input
        // vertex to the constructor. ToPlane(...) and ToCircumsphere(...)
        // are passed indices into the vertex array. When the vertex is
        // valid, mVertices[] and mCRVertices[] are used for lookups. When the
        // vertex is 'negOne', the query point is used for lookups.
        mutable Vector3<T> mQueryPoint;
        mutable Vector3<InputRational> mIRQueryPoint;

        // Sufficient storage for the expression trees related to computing
        // the exact signs in ToPlane(...) and ToCircumsphere(...).
        static size_t constexpr maxNumCRPool = 98;
        mutable std::vector<ComputeRational> mCRPool;
    };
}
