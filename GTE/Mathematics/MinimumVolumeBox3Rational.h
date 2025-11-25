// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.1.2025.11.24

#pragma once
#include <Mathematics/Logger.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/ConvexHull3.h>
#include <Mathematics/MinimumAreaBox2.h>
#include <Mathematics/MinimumVolumeBox3.h>
#include <Mathematics/UniqueVerticesSimplices.h>
#include <Mathematics/VETManifoldMesh.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <thread>
#include <type_traits>
#include <vector>

namespace gte
{
    // Type T is the floating-point type of the components; it must be 'float'
    // or 'double'. It might be necessary to increase the default program
    // stack size significantly, especially if the number of input points is
    // large. In Microsoft Visual Studio, I set the Stack Reserve Size to
    // 1 GB, which is 1073741824 bytes and is probably much more than
    // required.
    template <typename T, typename IndexType>
    class MinimumVolumeBox3<T, IndexType, MVB3Rational>
    {
    public:
        // Supporting constants and types for numerical computing.
        static int32_t constexpr NumWords = std::is_same<T, float>::value ? 342 : 2561;
        using UInteger = UIntegerFP32<NumWords>;
        using Number = BSNumber<UInteger>;
        using Rational = BSRational<UInteger>;
        using NVector3 = Vector3<Number>;
        using RVector3 = Vector3<Rational>;
        using TVector3 = Vector3<T>;

        // Construction and destruction. To execute in the main thread, set
        // numThreads to 0 or 1. To run multithreaded on the CPU, set
        // numThreads to 2 or larger.
        MinimumVolumeBox3(std::size_t numThreads = 0)
            :
            mNumThreads(numThreads),
            // topology
            mEdges{},
            mEdgeIndices{},
            mTriangles{},
            mAdjacentPool{},
            mAdjacentPoolLocation{},
            mVClimbStart(0),
            // geometry
            mNVertices{},
            mNNormals{},
            mNOrigin{},
            // minimization
            mAlignedCandidate{},
            mMinimumVolumeObject{},
            mMaxSample(0),
            mDomainIndex{},
            mLevelCurveProcessor{}
        {
            static_assert(std::is_floating_point<T>::value,
                "The input type must be 'float' or 'double'.");

            static_assert(
                std::is_integral<IndexType>::value &&
                !std::is_same<IndexType, bool>::value,
                "Invalid index type.");

            InitializeLevelCurveProcessors();
        }

        virtual ~MinimumVolumeBox3() = default;

        // Disallow copying and moving.
        MinimumVolumeBox3(MinimumVolumeBox3 const&) = delete;
        MinimumVolumeBox3& operator=(MinimumVolumeBox3 const&) = delete;
        MinimumVolumeBox3(MinimumVolumeBox3&&) noexcept = delete;
        MinimumVolumeBox3& operator=(MinimumVolumeBox3&&) = delete;

        // Compute the minimum volume box for an arbitrary set of points. The
        // returned std::size_t value is the dimension of the convex hull of
        // the points.
        std::size_t operator()(
            std::size_t numPoints,
            TVector3 const* points,
            std::size_t lgMaxSample,
            OrientedBox3<T>& box,
            T& volume)
        {
            // The vertices must be those for a 3-dimensional polytope. The
            // smallest such polytope is a tetrahedron, so there must be at
            // least 4 vertices and 4 triangles. The number of samples
            // must be at least 4.
            LogAssert(
                numPoints >= 4 && points != nullptr &&
                lgMaxSample >= 2,
                "Invalid argument.");

            std::size_t constexpr numConvexHullThreads = 0;
            std::vector<TVector3> hullVertices{};
            std::vector<IndexType> hullIndices{};
            std::size_t dimension = ComputeConvexHull(numConvexHullThreads,
                numPoints, points, hullVertices, hullIndices, box, volume);
            if (dimension == 3)
            {
                // Compute the minimum volume box for the 3D convex hull.
                operator()(
                    hullVertices.size(), hullVertices.data(),
                    hullIndices.size(), hullIndices.data(),
                    lgMaxSample, box, volume);
            }
            // else: The minimum volume box has volume zero. The number of
            // zero-valued box extents is 3-dimension.

            return dimension;
        }

        std::size_t operator()(
            std::vector<TVector3> const& points,
            std::size_t lgMaxSample,
            OrientedBox3<T>& box,
            T& volume)
        {
            return operator()(points.size(), points.data(), lgMaxSample, box, volume);
        }

        // The minimum volume box algorithm involves processing hyperbolic
        // curves. Each curve has a corresponding parameterization, Volume(s),
        // and the global minimum--or an approximation to it--must be
        // computed. The default method used for minimization is to compute
        // sample points along the curve and choosing that point which
        // provides the minimum among all samples. The number of samples is
        // 2^{lgMaxSample} for lgMaxSample >= 2; this implies there are at
        // least 4 samples. Do not choose lgMaxSample to be too large when
        // using rational computation because the computational costs are
        // excessive. You can override the minimizer functions to use your
        // own minimization algorithm; see the comments before the member
        // function MinimizerConstantT.
        //
        // The output is the minimum volume box and its volume, although
        // floating-point rounding errors can lead to a result that is nearly
        // the minimum volume box.
        void operator()(
            std::size_t numVertices,
            TVector3 const* vertices,
            std::size_t numIndices,
            IndexType const* indices,
            std::size_t lgMaxSample,
            OrientedBox3<T>& box,
            T& volume)
        {
            // The vertices must be those for a 3-dimensional polytope. The
            // smallest such polytope is a tetrahedron, so there must be at
            // least 4 vertices and 4 triangles. The number of samples
            // must be at least 4.
            LogAssert(
                numVertices >= 4 && vertices != nullptr &&
                numIndices >= 12 && (numIndices % 3) == 0 &&indices != nullptr &&
                lgMaxSample >= 2,
                "Invalid argument.");

            // Generate the 2^{lgMaxSample} sample points for minimizing the
            // volume along hyperbolic curves.
            GenerateSubdivision(lgMaxSample);

            // Create a vertex-edge-triangle graph and extract the topological
            // and geometric information from it.
            std::size_t const numTriangles = numIndices / 3;
            VETManifoldMesh mesh{};
            CreateMeshTopology(numTriangles, indices, mesh);
            ExtractMeshTopology(mesh);
            ExtractVertexAdjacencies(mesh);
            ExtractMeshGeometry(numVertices, vertices);

            // Given a mesh vertex V, the link polygon is a nonplanar and
            // closed polyline. Its vertices are immediately adjacent to V.
            // Remove those V for which the link polygon and V are coplanar.
            // Effectively, this is an implicit way to merge all triangles
            // that are coplanar and form a non-triangle face of the convex
            // hull. The GetExtreme function is a hill-climbing algorithm
            // and is the bottleneck in the MVB3 algorithm. Removing the
            // aforementioned V and disabling the relevant adjacent links
            // makes GetExtreme simple to implement. If they are not removed,
            // the logic for GetExtreme is more complicated and leads to a
            // massive performance loss for large point sets.
            RemoveCoplanarTriangleAdjacencies();

            // Start the search over pairs of normal vectors for the
            // configuration that leads to the minimum volume box.
            ComputeAlignedCandidate();
            GetMinimumVolumeCandidate();
            GetMinimumVolumeBox(box, volume);
        }

        void operator()(
            std::vector<TVector3> const& vertices,
            std::vector<IndexType> const& indices,
            std::size_t lgMaxSample,
            OrientedBox3<T>& box,
            T& volume)
        {
            operator()(vertices.size(), vertices.data(), indices.size(), indices.data(),
                lgMaxSample, box, volume);
        }

    protected:
        // The class members that store indices into the vertices use
        // std::size_t, not IndexType.
        static std::size_t constexpr invalidIndex = std::numeric_limits<std::size_t>::max();

        struct Edge
        {
            Edge()
                :
                v{ invalidIndex, invalidIndex },
                t{ invalidIndex, invalidIndex }
            {
            }
            std::array<std::size_t, 2> v;
            std::array<std::size_t, 2> t;
        };

        struct Triangle
        {
            Triangle()
                :
                v{ invalidIndex, invalidIndex },
                e{ invalidIndex, invalidIndex },
                t{ invalidIndex, invalidIndex }
            {
            }

            std::array<std::size_t, 3> v;
            std::array<std::size_t, 3> e;
            std::array<std::size_t, 3> t;
        };

        // Information about candidates for the minimum volume box and about
        // that box itself.
        struct Candidate
        {
            Candidate()
                :
                edgeIndex{ invalidIndex, invalidIndex },
                edge{},
                N{ NVector3::Zero(), NVector3::Zero() },
                M{ NVector3::Zero(), NVector3::Zero() },
                f00(static_cast<Number>(0)),
                f10(static_cast<Number>(0)),
                f01(static_cast<Number>(0)),
                f11(static_cast<Number>(0)),
                levelCurveProcessorIndex(invalidIndex),
                axis{ NVector3::Unit(0), NVector3::Unit(1), NVector3::Unit(2) },
                minSupportIndex{ invalidIndex, invalidIndex, invalidIndex },
                maxSupportIndex{ invalidIndex, invalidIndex, invalidIndex },
                volume(static_cast<Rational>(0))
            {
            }

            // Set by ProcessEdgePair.
            std::array<std::size_t, 2> edgeIndex;
            std::array<Edge, 2> edge;
            std::array<NVector3, 2> N, M;
            Number f00, f10, f01, f11;
            std::size_t levelCurveProcessorIndex;

            // Set by Pair, MinimizerConstantT, MinimizerConstantS,
            // MinimizerVariableS and MinimizerVariableT. The axis[0] and
            // axis[1] are set by the aforementioned functions. The axis[2]
            // is computed by ComputeVolume.
            std::array<NVector3, 3> axis;

            // Set by ComputeVolume.
            std::array<std::size_t, 3> minSupportIndex;
            std::array<std::size_t, 3> maxSupportIndex;
            Rational volume;
        };

        std::size_t ComputeConvexHull(
            std::size_t numConvexHullThreads,
            std::size_t numPoints, TVector3 const* points,
            std::vector<TVector3>& hullVertices, std::vector<IndexType>& hullIndices,
            OrientedBox3<T>& box, T& volume)
        {
            ConvexHull3<T> ch3{};
            ch3(numPoints, points, numConvexHullThreads);
            std::size_t dimension = ch3.GetDimension();
            auto const& hull = ch3.GetHull();

            if (dimension == 0)
            {
                // The points are all the same.
                T const zero = static_cast<T>(0);
                T const one = static_cast<T>(1);
                box.center = points[hull[0]];
                box.axis[0] = { one, zero, zero };
                box.axis[1] = { zero, one, zero };
                box.axis[2] = { zero, zero, one };
                box.extent[0] = zero;
                box.extent[1] = zero;
                box.extent[2] = zero;
                volume = zero;
            }
            else if (dimension == 1)
            {
                // The points lie on a line.
                T const zero = static_cast<T>(0);
                T const half = static_cast<T>(0.5);
                TVector3 direction = points[hull[1]] - points[hull[0]];
                box.center = half * (points[hull[0]] + points[hull[1]]);
                box.extent[0] = half * Normalize(direction);
                box.extent[1] = zero;
                box.extent[2] = zero;
                box.axis[0] = direction;
                ComputeOrthogonalComplement(1, box.axis.data());
                volume = zero;
            }
            else if (dimension == 2)
            {
                // The points line on a plane. Get a coordinate system
                // relative to the plane of the points. Choose the origin
                // to be any of the input points.
                TVector3 origin = points[hull[0]];
                TVector3 normal = TVector3::Zero();
                std::size_t numHull = hull.size();
                for (std::size_t i0 = numHull - 1, i1 = 1; i1 < numHull; i0 = i1++)
                {
                    auto const& P0 = points[hull[i0]];
                    auto const& P1 = points[hull[i1]];
                    normal += Cross(P0, P1);
                }

                std::array<TVector3, 3> basis{};
                basis[0] = normal;
                ComputeOrthogonalComplement(1, basis.data());

                // Project the input points onto the plane.
                std::vector<Vector2<T>> projection(numPoints);
                for (std::size_t i = 0; i < numPoints; ++i)
                {
                    TVector3 diff = points[i] - origin;
                    projection[i][0] = Dot(basis[1], diff);
                    projection[i][1] = Dot(basis[2], diff);
                }

                // Compute the minimum area box in 2D.
                MinimumAreaBox2<T, T> mab2{};
                OrientedBox2<T> rectangle = mab2(static_cast<std::int32_t>(numPoints), projection.data());

                // Lift the values into 3D.
                T const zero = static_cast<T>(0);
                box.center = origin + rectangle.center[0] * basis[1] + rectangle.center[1] * basis[2];
                box.axis[0] = rectangle.axis[0][0] * basis[1] + rectangle.axis[0][1] * basis[2];
                box.axis[1] = rectangle.axis[1][0] * basis[1] + rectangle.axis[1][1] * basis[2];
                box.axis[2] = basis[0];
                box.extent[0] = rectangle.extent[0];
                box.extent[1] = rectangle.extent[1];
                box.extent[2] = zero;
                volume = zero;
            }
            else // dimension == 3
            {
                // Remove duplicated vertices and reindex them for the polytope.
                std::vector<TVector3> sourceVertices(numPoints);
                std::memcpy(sourceVertices.data(), points, sourceVertices.size() * sizeof(TVector3));
                auto const& ch3Indices = ch3.GetHull();
                std::vector<IndexType> sourceIndices(ch3Indices.size());
                std::size_t current = 0;
                for (auto index : ch3Indices)
                {
                    sourceIndices[current++] = static_cast<IndexType>(index);
                }

                UniqueVerticesSimplices<TVector3, IndexType, 3> uvt{};
                uvt.RemoveDuplicateAndUnusedVertices(
                    sourceVertices, sourceIndices,
                    hullVertices, hullIndices);
            }

            return dimension;
        }

        void CreateMeshTopology(std::size_t numTriangles, IndexType const* indices, VETManifoldMesh& mesh)
        {
            IndexType const* current = indices;
            for (std::size_t t = 0; t < numTriangles; ++t)
            {
                std::int32_t v0 = static_cast<std::int32_t>(*current);
                ++current;
                std::int32_t v1 = static_cast<std::int32_t>(*current);
                ++current;
                std::int32_t v2 = static_cast<std::int32_t>(*current);
                ++current;
                mesh.Insert(v0, v1, v2);
            }

            auto const& vMap = mesh.GetVertices();
            auto const& eMap = mesh.GetEdges();
            auto const& tMap = mesh.GetTriangles();
            mEdges.resize(eMap.size());
            mEdgeIndices.reserve(eMap.size() * eMap.size());
            mTriangles.resize(tMap.size());
            mNVertices.resize(vMap.size());
            mNNormals.resize(tMap.size());
        }

        void ExtractMeshTopology(VETManifoldMesh const& mesh)
        {
            auto const& eMap = mesh.GetEdges();
            auto const& tMap = mesh.GetTriangles();

            std::map<ETManifoldMesh::Edge*, std::size_t> edgeIndexMap{};
            std::size_t index = 0;
            for (auto const& element : eMap)
            {
                edgeIndexMap.emplace(element.second.get(), index);
                for (std::size_t j = 0; j < 2; ++j)
                {
                    mEdges[index].v[j] = static_cast<std::size_t>(element.second->V[j]);
                }
                ++index;
            }

            std::map<ETManifoldMesh::Triangle*, std::size_t> triangleIndexMap{};
            index = 0;
            for (auto const& element : tMap)
            {
                triangleIndexMap.emplace(element.second.get(), index);
                for (std::size_t j = 0; j < 3; ++j)
                {
                    mTriangles[index].v[j] = static_cast<std::size_t>(element.second->V[j]);
                }
                ++index;
            }

            index = 0;
            for (auto const& element : eMap)
            {
                for (std::size_t j = 0; j < 2; ++j)
                {
                    auto triangleIndex = element.second->T[j];
                    auto tIter = triangleIndexMap.find(triangleIndex);
                    mEdges[index].t[j] = tIter->second;
                }
                ++index;
            }

            index = 0;
            for (auto const& element : tMap)
            {
                for (std::size_t j = 0; j < 3; ++j)
                {
                    auto edgeIndex = element.second->E[j];
                    auto eIter = edgeIndexMap.find(edgeIndex);
                    mTriangles[index].e[j] = eIter->second;
                }
                for (std::size_t j = 0; j < 3; ++j)
                {
                    auto triangleIndex = element.second->T[j];
                    auto tIter = triangleIndexMap.find(triangleIndex);
                    mTriangles[index].t[j] = tIter->second;
                }
                ++index;
            }

            for (std::size_t e0 = 0; e0 < mEdges.size(); ++e0)
            {
                for (std::size_t e1 = e0 + 1; e1 < mEdges.size(); ++e1)
                {
                    mEdgeIndices.push_back({ e0, e1 });
                }
            }
        }

        void ExtractVertexAdjacencies(VETManifoldMesh const& mesh)
        {
            // The vertices are stored in a vertex-edge-triangle manifold
            // mesh. Each vertex as a set of adjacent vertices, a set of
            // adjacent edges and a set of adjacent triangles. The adjacent
            // vertices are repackaged into mAdjacentPoolLocation[] and
            // mAdjacentPool[]. For vertex v with n adjacent vertices,
            // mAdjacentPoolLocation[v] is the index into mAdjacentPool[]
            // where the n adjacent vertices are stored. If the adjacent
            // vertices are a[0] through a[n-1], then
            // mAdjacentPool[mAdjacentPoolLocation[v] + i] is a[i] for
            // 0 <= i < n.

            // Create the vertex-adjacency information for each vertex of the
            // polytope. In the construction of mAdjacentPoolLocation[v]:
            //   (1) the vertex indices v satisfy 0 <= v < N for a mesh of N
            //       vertices and
            //   (2) the vertex map itself is ordered as <0,vertex0>,
            //       <1,vertex1>, ..., <N-1,vertexNm1>.
            // Condition (1) is guaranteed because the input to the MVB3
            // constructor uses the contiguous indices of the position array.
            // Condition (2) is not guaranteed because VETManifoldMesh::VMap
            // is a std::unordered_map. The vertices must be sorted here to
            // satisfy condition (2).
            std::map<std::int32_t, VETManifoldMesh::Vertex*> sortedVMap{};
            auto const& vMap = mesh.GetVertices();
            for (auto const& element : vMap)
            {
                sortedVMap.emplace(element.first, element.second.get());
            }

            std::size_t numAdjacentPool = 0;
            for (auto const& element : sortedVMap)
            {
                numAdjacentPool += element.second->VAdjacent.size() + 1;
            }
            mAdjacentPool.resize(numAdjacentPool);
            mAdjacentPoolLocation.resize(sortedVMap.size());
            std::size_t apIndex = 0, vaIndex = 0;
            for (auto const& element : sortedVMap)
            {
                auto const& adjacent = element.second->VAdjacent;
                mAdjacentPoolLocation[vaIndex++] = apIndex;
                mAdjacentPool[apIndex++] = adjacent.size();
                for (auto v : adjacent)
                {
                    mAdjacentPool[apIndex++] = static_cast<std::size_t>(v);
                }
            }
        }

        void ExtractMeshGeometry(std::size_t numVertices, TVector3 const* vertices)
        {
            // Translate the polytope so that vertices[0] becomes the origin.
            // This helps avoid large floating-point rounding errors when the
            // polytope is far away from (0,0,0).
            for (std::int32_t j = 0; j < 3; ++j)
            {
                mNOrigin[j] = static_cast<Number>(vertices[0][j]);
            }
            mNVertices[0] = NVector3::Zero();
            for (std::size_t i = 1; i < numVertices; ++i)
            {
                for (std::int32_t j = 0; j < 3; ++j)
                {
                    mNVertices[i][j] = static_cast<Number>(vertices[i][j]) - mNOrigin[j];
                }
            }

            // Create the triangles and normals to the triangles. The normals
            // are not normalized to avoid floating-point rounding errors.
            // This is necessary for creating the vertex adjacency data
            // structure that supports the hill-climbing algorithm that
            // computes extreme hull points in a specified direction. The
            // hill climbing is implemented in GetExtreme(...).
            for (std::size_t i = 0; i < mTriangles.size(); ++i)
            {
                auto const& tri = mTriangles[i];
                std::size_t v0 = tri.v[0], v1 = tri.v[1], v2 = tri.v[2];
                NVector3 edge10 = mNVertices[v1] - mNVertices[v0];
                NVector3 edge20 = mNVertices[v2] - mNVertices[v0];
                mNNormals[i] = Cross(edge20, edge10);
            }
        }

        void InsertAdjacent(std::size_t vertex, std::size_t insertionCandidate)
        {
            auto* adjacent = &mAdjacentPool[mAdjacentPoolLocation[vertex]];
            auto& numAdjacent = adjacent[0];
            ++numAdjacent;
            adjacent[numAdjacent] = insertionCandidate;
        }

        void RemoveAdjacent(std::size_t vertex, std::size_t removalCandidate)
        {
            auto* adjacent = &mAdjacentPool[mAdjacentPoolLocation[vertex]];
            auto& numAdjacent = adjacent[0];
            for (std::size_t j = 1; j <= numAdjacent; ++j)
            {
                if (adjacent[j] == removalCandidate)
                {
                    // The adjacent candidate is indeed adjacent to the
                    // vertex, so remove it. To maintain a contiguous array of
                    // adjacents, move the last element of the array to the
                    // location vacated by the adjacent candidate. If the
                    // vacated location is already the end of the array, there
                    // is nothing to move.
                    if (j < numAdjacent)
                    {
                        adjacent[j] = adjacent[numAdjacent];
                    }
                    adjacent[numAdjacent] = invalidIndex;
                    --numAdjacent;
                    return;
                }
            }
        }

        void RemoveCoplanarTriangleAdjacencies()
        {
            // Adjacent triangles are coplanar if their unit-length normal
            // vectors are equal. For such triangles, the winding order of the
            // triangles in the manifold mesh guarantees the normals point in
            // the same direction; that is, we cannot have N1 = -N0.
            for (auto const& edge : mEdges)
            {
                auto const& N0 = mNNormals[edge.t[0]];
                auto const& N1 = mNNormals[edge.t[1]];
                auto N0xN1 = Cross(N0, N1);
                if (N0xN1 == NVector3::Zero())
                {
                    // The triangles sharing the edge are coplanar. Remove the
                    // vertex-adjacent information for the edge vertices. This
                    // leads to an implied removal of coplanar triangles which
                    // then makes the GetExtreme hill-climbing algorithm 
                    // simple to implement by not having to keep track of the
                    // bookkeeping while traversinh a patch of coplanar
                    // vertices.
                    RemoveAdjacent(edge.v[0], edge.v[1]);
                    RemoveAdjacent(edge.v[1], edge.v[0]);
                }
            }

            // After removing interior edges of a coplanar triangle face,
            // the boundary edges of the face can have colinear vertices.
            // These vertices must be removed so that the face becomes a
            // convex polygon with no colinear vertices.
            NVector3 zero{};
            for (std::size_t v = 0; v < mNVertices.size(); ++v)
            {
                std::size_t adjLoc = mAdjacentPoolLocation[v];
                auto* adjacent = &mAdjacentPool[adjLoc];
                std::size_t& numAdjacent = adjacent[0];
                if (numAdjacent == 2)
                {
                    // Test for colinearity.
                    std::size_t vPrev = adjacent[1];
                    std::size_t vNext = adjacent[2];
                    auto diff0 = mNVertices[v] - mNVertices[vPrev];
                    auto diff1 = mNVertices[v] - mNVertices[vNext];
                    auto cross = Cross(diff0, diff1);
                    if (cross == zero)
                    {
                        // The points are colinear. Remove the middle point.
                        RemoveAdjacent(v, vPrev);
                        RemoveAdjacent(vPrev, v);
                        RemoveAdjacent(v, vNext);
                        RemoveAdjacent(vNext, v);
                        numAdjacent = 0;

                        // The endpoints are now adjacent.
                        InsertAdjacent(vPrev, vNext);
                        InsertAdjacent(vNext, vPrev);
                    }
                }
            }

            // Locate the first nonempty adjacency list and use it to set the
            // initial index into mAdjacentPoolLocation[] for the hill
            // climbing.
            mVClimbStart = invalidIndex;
            for (std::size_t i = 0; i < mNVertices.size(); ++i)
            {
                std::size_t numAdjacent = mAdjacentPool[mAdjacentPoolLocation[i]];
                if (numAdjacent > 0)
                {
                    mVClimbStart = i;
                    break;
                }
            }

            LogAssert(
                mVClimbStart != invalidIndex,
                "Unexpected condition: At least one adjacency list should be nonempty.");
        }

        void ComputeAlignedCandidate()
        {
            NVector3 pmin{}, pmax{};
            for (std::int32_t j = 0; j < 3; ++j)
            {
                mAlignedCandidate.maxSupportIndex[j] = GetExtreme(+mAlignedCandidate.axis[j], pmax[j]);
                mAlignedCandidate.minSupportIndex[j] = GetExtreme(-mAlignedCandidate.axis[j], pmin[j]);
                pmin[j] = -pmin[j];
            }
            NVector3 diff = pmax - pmin;
            mAlignedCandidate.volume = diff[0] * diff[1] * diff[2];
        }

        std::size_t GetExtreme(NVector3 const& direction, Number& dMax)
        {
            std::size_t vMax = mVClimbStart;
            dMax = Dot(direction, mNVertices[vMax]);

            for (std::size_t i = 0; i < mNVertices.size(); ++i)
            {
                std::size_t vLocalMax = vMax;
                Number dLocalMax = dMax;
                std::size_t const* adjacent = &mAdjacentPool[mAdjacentPoolLocation[vMax]];
                std::size_t numAdjacent = *adjacent++;
                for (std::size_t j = 1; j <= numAdjacent; ++j)
                {
                    std::size_t vCandidate = *adjacent++;
                    Number dCandidate = Dot(direction, mNVertices[vCandidate]);
                    if (dCandidate > dLocalMax)
                    {
                        vLocalMax = vCandidate;
                        dLocalMax = dCandidate;
                    }
                }
                if (vMax != vLocalMax)
                {
                    vMax = vLocalMax;
                    dMax = dLocalMax;
                }
                else
                {
                    break;
                }
            }

            return vMax;
        }

        void ComputeVolume(Candidate& candidate)
        {
            // The last axis is needed only when computing the volume for
            // comparison to the current candidate volume, so compute this
            // axis now.
            candidate.axis[2] = Cross(candidate.axis[0], candidate.axis[1]);

            NVector3 pmin{}, pmax{};
            candidate.minSupportIndex[0] = mEdges[candidate.edgeIndex[0]].v[0];
            pmin[0] = Dot(candidate.axis[0], mNVertices[candidate.minSupportIndex[0]]);
            candidate.maxSupportIndex[0] = GetExtreme(candidate.axis[0], pmax[0]);
            candidate.minSupportIndex[1] = mEdges[candidate.edgeIndex[1]].v[0];
            pmin[1] = Dot(candidate.axis[1], mNVertices[candidate.minSupportIndex[1]]);
            candidate.maxSupportIndex[1] = GetExtreme(candidate.axis[1], pmax[1]);
            candidate.axis[2] = Cross(candidate.axis[0], candidate.axis[1]);
            candidate.minSupportIndex[2] = GetExtreme(-candidate.axis[2], pmin[2]);
            pmin[2] = -pmin[2];
            candidate.maxSupportIndex[2] = GetExtreme(candidate.axis[2], pmax[2]);
            NVector3 diff = pmax - pmin;
            candidate.volume = static_cast<Rational>(diff[0] * diff[1] * diff[2]) /
                static_cast<Rational>(Dot(candidate.axis[2], candidate.axis[2]));
        }

        void ProcessEdgePair(std::array<std::size_t, 2> const& edgeIndex, Candidate& mvCandidate)
        {
            // Examine the zero-valued level curves for
            // F(s,t)
            // = Dot((1-s)*edge0.N0 + s*edge0.N1, (1-t)*edge1.N0 + t*edge1.N1)
            // = (1-s)*(1-t)*Dot(edge0.N0,edge1.N0)
            //   + (1-s)*t*Dot(edge0.N0,edge1.N1)
            //   + s*(1-t)*Dot(edge0.N1,edge1.N0)
            //   + s*t*Dot(edge0.N1,edge1.N1)
            // = (1-s)*(1-t)*f00 + (1-s)*t*f01 + s*(1-t)*f10 + s*t*f11
            // = a00 + a10*s + a01*t + a11*s*t
            // = [(a00*a11 - a01*a10) + (a01 + a11*s)*(a10 + a11*t)]/a11
            // where a00 = f00, a10 = f10-f00, a01 = f01-f00 and
            // a11 = f00-f01-f10+f11.  Let d = a00*a11 - a01*a10 =
            // f00*f11 - f01*f10. If d = 0, then the level curves are
            // s = -a01/a11 and t = -a10/a11. If d != 0, then the level curves
            // are hyperbolic curves with asymptotes s = -a01/a11 and
            // t = -a10/a11.

            Candidate candidate = mAlignedCandidate;
            candidate.edgeIndex = edgeIndex;
            Edge const& edge0 = mEdges[candidate.edgeIndex[0]];
            Edge const& edge1 = mEdges[candidate.edgeIndex[1]];
            candidate.edge[0] = edge0;
            candidate.edge[1] = edge1;
            candidate.N[0] = mNNormals[edge0.t[0]];
            candidate.N[1] = mNNormals[edge0.t[1]];
            candidate.M[0] = mNNormals[edge1.t[0]];
            candidate.M[1] = mNNormals[edge1.t[1]];
            candidate.f00 = Dot(candidate.N[0], candidate.M[0]);
            candidate.f10 = Dot(candidate.N[1], candidate.M[0]);
            candidate.f01 = Dot(candidate.N[0], candidate.M[1]);
            candidate.f11 = Dot(candidate.N[1], candidate.M[1]);

            Number const zero = static_cast<Number>(0);
            std::uint32_t bits00 = (candidate.f00 > zero ? 1 : (candidate.f00 < zero ? 2 : 0));
            std::uint32_t bits10 = (candidate.f10 > zero ? 1 : (candidate.f10 < zero ? 2 : 0));
            std::uint32_t bits01 = (candidate.f01 > zero ? 1 : (candidate.f01 < zero ? 2 : 0));
            std::uint32_t bits11 = (candidate.f11 > zero ? 1 : (candidate.f11 < zero ? 2 : 0));
            std::uint32_t index = bits00 | (bits10 << 2) | (bits01 << 4) | (bits11 << 6);
            if (index != 0x55 && index != 0xaa)
            {
                candidate.levelCurveProcessorIndex = index;
                (this->*mLevelCurveProcessor[candidate.levelCurveProcessorIndex])(candidate, mvCandidate);
            }
        }

        void GetMinimumVolumeCandidate()
        {
            mMinimumVolumeObject = mAlignedCandidate;

            if (mNumThreads > 0)
            {
                std::size_t const numPairsPerThread = mEdgeIndices.size() / mNumThreads;
                std::vector<std::size_t> imin(mNumThreads), imax(mNumThreads);
                for (std::size_t t = 0; t < mNumThreads; ++t)
                {
                    imin[t] = t * numPairsPerThread;
                    imax[t] = (t + 1) * numPairsPerThread;
                }
                imax.back() = mEdgeIndices.size();

                std::vector<Candidate> candidates(mNumThreads);
                std::vector<std::thread> process(mNumThreads);
                for (std::size_t t = 0; t < mNumThreads; ++t)
                {
                    process[t] = std::thread(
                        [this, t, &imin, &imax, &candidates]()
                        {
                            candidates[t] = mAlignedCandidate;
                            for (std::size_t i = imin[t]; i < imax[t]; ++i)
                            {
                                ProcessEdgePair(mEdgeIndices[i], candidates[t]);
                            }
                        });
                }

                for (std::size_t t = 0; t < mNumThreads; ++t)
                {
                    process[t].join();
                    if (candidates[t].volume < mMinimumVolumeObject.volume)
                    {
                        mMinimumVolumeObject = candidates[t];
                    }
                }
            }
            else
            {
                for (auto const& edgeIndex : mEdgeIndices)
                {
                    ProcessEdgePair(edgeIndex, mMinimumVolumeObject);
                }
            }
        }

        void GetMinimumVolumeBox(OrientedBox3<T>& box, T& volume)
        {
            Candidate const& mvc = mMinimumVolumeObject;

            // Compute the rational-valued box and volume. Convert this to a
            // floating-point-valued box and volume on return.
            RVector3 rCenter{}, rPMin{}, rPMax{};
            std::array<RVector3, 3> rAxis{};
            std::array<Rational, 3> rSqrLengthAxis{};
            for (std::int32_t i = 0; i < 3; ++i)
            {
                rCenter[i] = mNOrigin[i];

                for (std::int32_t j = 0; j < 3; ++j)
                {
                    rAxis[i][j] = mvc.axis[i][j];
                }
                rSqrLengthAxis[i] = Dot(rAxis[i], rAxis[i]);

                rPMin[i] = Dot(mvc.axis[i], mNVertices[mvc.minSupportIndex[i]]);
                rPMax[i] = Dot(mvc.axis[i], mNVertices[mvc.maxSupportIndex[i]]);
            }

            Rational const rHalf(0.5);
            RVector3 rAverage = rHalf * (rPMax + rPMin);
            for (std::int32_t i = 0; i < 3; ++i)
            {
                for (std::int32_t j = 0; j < 3; ++j)
                {
                    rCenter[j] += (rAverage[i] / rSqrLengthAxis[i]) * rAxis[i][j];
                }
            }

            RVector3 rDifference = rPMax - rPMin;
            RVector3 rScaledExtent = rHalf * rDifference;
            Rational rVolume = rDifference[0] * rDifference[1] * rDifference[2] / rSqrLengthAxis[2];

            // Compute the floating-point-valued box and volume.
            for (std::int32_t i = 0; i < 3; ++i)
            {
                box.center[i] = static_cast<T>(rCenter[i]);
                T length = static_cast<T>(std::sqrt(rSqrLengthAxis[i]));
                for (std::int32_t j = 0; j < 3; ++j)
                {
                    box.axis[i][j] = static_cast<T>(rAxis[i][j]) / length;
                }
                box.extent[i] = static_cast<T>(rScaledExtent[i]) / length;
            }
            volume = static_cast<T>(rVolume);
        }

        // The number of threads to use for computing. If 0 or 1, the main
        // thread is used. If 2 or larger, std::thread objects are used.
        std::size_t mNumThreads;

        // A mesh representation of the polytope. These members store
        // topological information.
        std::vector<Edge> mEdges;
        std::vector<std::array<std::size_t, 2>> mEdgeIndices;
        std::vector<Triangle> mTriangles;
        std::vector<std::size_t> mAdjacentPool;
        std::vector<std::size_t> mAdjacentPoolLocation;
        std::size_t mVClimbStart;

        // These members store geometric information.
        std::vector<NVector3> mNVertices;
        std::vector<NVector3> mNNormals;
        NVector3 mNOrigin;

        // The axis-aligned bounding box of the vertices is used as the
        // initial candidate for the minimum-volume box.
        Candidate mAlignedCandidate;

        // The information for the minimum-volume bounding box of the
        // vertices.
        Candidate mMinimumVolumeObject;

    protected:
        // The maximum sample index used to search each level curve for
        // non-face-supporting boxes (mMaxSample + 1 values). The samples are
        // visited using subdivision of the domain of the level curve. The
        // subdivision information is stored in mDomainIndex(mNumSamples-1).
        std::size_t mMaxSample;
        std::vector<std::array<std::size_t, 3>> mDomainIndex;

        // Each member function A00B10C01D11(*) corresponds to a bilinear
        // function on the domain [0,1]^2. Each corner of the domain has a
        // bilinear function value that is positive, negative or zero,
        // leading to 3^4 = 81 possibilities. The 'A', 'B', 'C' and 'D' are
        // in {'P', 'M', 'Z'} [for Plus, Minus, Zero].
        typedef void (MinimumVolumeBox3::* LevelCurveProcessor)(Candidate&, Candidate&);
        std::array<LevelCurveProcessor, 256> mLevelCurveProcessor;

        void CreateDomainIndex(std::size_t& current, std::size_t end0, std::size_t end1)
        {
            std::size_t mid = (end0 + end1) / 2;
            if (mid != end0 && mid != end1)
            {
                mDomainIndex[current++] = { mid, end0, end1 };
                CreateDomainIndex(current, end0, mid);
                CreateDomainIndex(current, mid, end1);
            }
        }

        void GenerateSubdivision(std::size_t lgMaxSample)
        {
            mMaxSample = (static_cast<std::size_t>(1) << lgMaxSample);
            mDomainIndex.resize(mMaxSample - 1);
            std::size_t current = 0;
            CreateDomainIndex(current, 0, mMaxSample);
        }

        // Support for the level-curve processing functions.
        void InitializeLevelCurveProcessors()
        {
            // Generate the initialization code for mLevelCurveProcessor.
            // To compile the code, include <fstream>, <strstream> and
            // <iomanip.
            //
            // std::ofstream output("LevelCurveProcessor.txt");
            // std::array<char, 3> signchar = { 'Z', 'P', 'M' };
            // for (std::uint32_t index = 0; index < 256u; ++index)
            // {
            //     if ((index & 0x00000003u) != 0x00000003u &&
            //         (index & 0x0000000Cu) != 0x0000000Cu &&
            //         (index & 0x00000030u) != 0x00000030u &&
            //         (index & 0x000000C0u) != 0x000000C0u)
            //     {
            //         char s00 = signchar[index & 0x00000003u];
            //         char s10 = signchar[(index & 0x0000000Cu) >> 2];
            //         char s01 = signchar[(index & 0x00000030u) >> 4];
            //         char s11 = signchar[(index & 0x000000C0u) >> 6];
            //         std::strstream ostream;
            //         ostream << std::hex << std::setfill('0') << std::setw(2);
            //         ostream
            //             << "    mLevelCurveProcessor[0x0"
            //             << std::hex << std::setfill('0') << std::setw(2)
            //             << index
            //             << "] = &MinimumVolumeBox3::"
            //             << s00 << "00"
            //             << s10 << "10"
            //             << s01 << "01"
            //             << s11 << "11;"
            //             << std::ends;
            //         output << ostream.str() << std::endl;
            //     }
            // }
            // output.close();

            mLevelCurveProcessor.fill(nullptr);
            mLevelCurveProcessor[0x00] = &MinimumVolumeBox3::Z00Z10Z01Z11;
            mLevelCurveProcessor[0x01] = &MinimumVolumeBox3::P00Z10Z01Z11;
            mLevelCurveProcessor[0x02] = &MinimumVolumeBox3::M00Z10Z01Z11;
            mLevelCurveProcessor[0x04] = &MinimumVolumeBox3::Z00P10Z01Z11;
            mLevelCurveProcessor[0x05] = &MinimumVolumeBox3::P00P10Z01Z11;
            mLevelCurveProcessor[0x06] = &MinimumVolumeBox3::M00P10Z01Z11;
            mLevelCurveProcessor[0x08] = &MinimumVolumeBox3::Z00M10Z01Z11;
            mLevelCurveProcessor[0x09] = &MinimumVolumeBox3::P00M10Z01Z11;
            mLevelCurveProcessor[0x0a] = &MinimumVolumeBox3::M00M10Z01Z11;
            mLevelCurveProcessor[0x10] = &MinimumVolumeBox3::Z00Z10P01Z11;
            mLevelCurveProcessor[0x11] = &MinimumVolumeBox3::P00Z10P01Z11;
            mLevelCurveProcessor[0x12] = &MinimumVolumeBox3::M00Z10P01Z11;
            mLevelCurveProcessor[0x14] = &MinimumVolumeBox3::Z00P10P01Z11;
            mLevelCurveProcessor[0x15] = &MinimumVolumeBox3::P00P10P01Z11;
            mLevelCurveProcessor[0x16] = &MinimumVolumeBox3::M00P10P01Z11;
            mLevelCurveProcessor[0x18] = &MinimumVolumeBox3::Z00M10P01Z11;
            mLevelCurveProcessor[0x19] = &MinimumVolumeBox3::P00M10P01Z11;
            mLevelCurveProcessor[0x1a] = &MinimumVolumeBox3::M00M10P01Z11;
            mLevelCurveProcessor[0x20] = &MinimumVolumeBox3::Z00Z10M01Z11;
            mLevelCurveProcessor[0x21] = &MinimumVolumeBox3::P00Z10M01Z11;
            mLevelCurveProcessor[0x22] = &MinimumVolumeBox3::M00Z10M01Z11;
            mLevelCurveProcessor[0x24] = &MinimumVolumeBox3::Z00P10M01Z11;
            mLevelCurveProcessor[0x25] = &MinimumVolumeBox3::P00P10M01Z11;
            mLevelCurveProcessor[0x26] = &MinimumVolumeBox3::M00P10M01Z11;
            mLevelCurveProcessor[0x28] = &MinimumVolumeBox3::Z00M10M01Z11;
            mLevelCurveProcessor[0x29] = &MinimumVolumeBox3::P00M10M01Z11;
            mLevelCurveProcessor[0x2a] = &MinimumVolumeBox3::M00M10M01Z11;
            mLevelCurveProcessor[0x40] = &MinimumVolumeBox3::Z00Z10Z01P11;
            mLevelCurveProcessor[0x41] = &MinimumVolumeBox3::P00Z10Z01P11;
            mLevelCurveProcessor[0x42] = &MinimumVolumeBox3::M00Z10Z01P11;
            mLevelCurveProcessor[0x44] = &MinimumVolumeBox3::Z00P10Z01P11;
            mLevelCurveProcessor[0x45] = &MinimumVolumeBox3::P00P10Z01P11;
            mLevelCurveProcessor[0x46] = &MinimumVolumeBox3::M00P10Z01P11;
            mLevelCurveProcessor[0x48] = &MinimumVolumeBox3::Z00M10Z01P11;
            mLevelCurveProcessor[0x49] = &MinimumVolumeBox3::P00M10Z01P11;
            mLevelCurveProcessor[0x4a] = &MinimumVolumeBox3::M00M10Z01P11;
            mLevelCurveProcessor[0x50] = &MinimumVolumeBox3::Z00Z10P01P11;
            mLevelCurveProcessor[0x51] = &MinimumVolumeBox3::P00Z10P01P11;
            mLevelCurveProcessor[0x52] = &MinimumVolumeBox3::M00Z10P01P11;
            mLevelCurveProcessor[0x54] = &MinimumVolumeBox3::Z00P10P01P11;
            mLevelCurveProcessor[0x55] = &MinimumVolumeBox3::P00P10P01P11;
            mLevelCurveProcessor[0x56] = &MinimumVolumeBox3::M00P10P01P11;
            mLevelCurveProcessor[0x58] = &MinimumVolumeBox3::Z00M10P01P11;
            mLevelCurveProcessor[0x59] = &MinimumVolumeBox3::P00M10P01P11;
            mLevelCurveProcessor[0x5a] = &MinimumVolumeBox3::M00M10P01P11;
            mLevelCurveProcessor[0x60] = &MinimumVolumeBox3::Z00Z10M01P11;
            mLevelCurveProcessor[0x61] = &MinimumVolumeBox3::P00Z10M01P11;
            mLevelCurveProcessor[0x62] = &MinimumVolumeBox3::M00Z10M01P11;
            mLevelCurveProcessor[0x64] = &MinimumVolumeBox3::Z00P10M01P11;
            mLevelCurveProcessor[0x65] = &MinimumVolumeBox3::P00P10M01P11;
            mLevelCurveProcessor[0x66] = &MinimumVolumeBox3::M00P10M01P11;
            mLevelCurveProcessor[0x68] = &MinimumVolumeBox3::Z00M10M01P11;
            mLevelCurveProcessor[0x69] = &MinimumVolumeBox3::P00M10M01P11;
            mLevelCurveProcessor[0x6a] = &MinimumVolumeBox3::M00M10M01P11;
            mLevelCurveProcessor[0x80] = &MinimumVolumeBox3::Z00Z10Z01M11;
            mLevelCurveProcessor[0x81] = &MinimumVolumeBox3::P00Z10Z01M11;
            mLevelCurveProcessor[0x82] = &MinimumVolumeBox3::M00Z10Z01M11;
            mLevelCurveProcessor[0x84] = &MinimumVolumeBox3::Z00P10Z01M11;
            mLevelCurveProcessor[0x85] = &MinimumVolumeBox3::P00P10Z01M11;
            mLevelCurveProcessor[0x86] = &MinimumVolumeBox3::M00P10Z01M11;
            mLevelCurveProcessor[0x88] = &MinimumVolumeBox3::Z00M10Z01M11;
            mLevelCurveProcessor[0x89] = &MinimumVolumeBox3::P00M10Z01M11;
            mLevelCurveProcessor[0x8a] = &MinimumVolumeBox3::M00M10Z01M11;
            mLevelCurveProcessor[0x90] = &MinimumVolumeBox3::Z00Z10P01M11;
            mLevelCurveProcessor[0x91] = &MinimumVolumeBox3::P00Z10P01M11;
            mLevelCurveProcessor[0x92] = &MinimumVolumeBox3::M00Z10P01M11;
            mLevelCurveProcessor[0x94] = &MinimumVolumeBox3::Z00P10P01M11;
            mLevelCurveProcessor[0x95] = &MinimumVolumeBox3::P00P10P01M11;
            mLevelCurveProcessor[0x96] = &MinimumVolumeBox3::M00P10P01M11;
            mLevelCurveProcessor[0x98] = &MinimumVolumeBox3::Z00M10P01M11;
            mLevelCurveProcessor[0x99] = &MinimumVolumeBox3::P00M10P01M11;
            mLevelCurveProcessor[0x9a] = &MinimumVolumeBox3::M00M10P01M11;
            mLevelCurveProcessor[0xa0] = &MinimumVolumeBox3::Z00Z10M01M11;
            mLevelCurveProcessor[0xa1] = &MinimumVolumeBox3::P00Z10M01M11;
            mLevelCurveProcessor[0xa2] = &MinimumVolumeBox3::M00Z10M01M11;
            mLevelCurveProcessor[0xa4] = &MinimumVolumeBox3::Z00P10M01M11;
            mLevelCurveProcessor[0xa5] = &MinimumVolumeBox3::P00P10M01M11;
            mLevelCurveProcessor[0xa6] = &MinimumVolumeBox3::M00P10M01M11;
            mLevelCurveProcessor[0xa8] = &MinimumVolumeBox3::Z00M10M01M11;
            mLevelCurveProcessor[0xa9] = &MinimumVolumeBox3::P00M10M01M11;
            mLevelCurveProcessor[0xaa] = &MinimumVolumeBox3::M00M10M01M11;
        }

        void Pair(Candidate& c, Candidate& mvc)
        {
            ComputeVolume(c);
            if (c.volume < mvc.volume)
            {
                mvc = c;
            }
        }

        // The minimizers for the operator()(maxSample, *) function. The
        // default behavior of MinimumVolumeBox3D is to use the built-in
        // minimizers that sample the level curves as a simple search for a
        // minimum volume. However, you can override the minimizers and
        // provide a more sophisticated algorithm.
        virtual void MinimizerConstantS(Candidate& c, Candidate& mvc)
        {
            Number const half = static_cast<Number>(0.5);
            std::vector<Number> t(mMaxSample + 1);
            t[0] = static_cast<Number>(0);
            t[mMaxSample] = static_cast<Number>(1);
            for (auto const& item : mDomainIndex)
            {
                t[item[0]] = half * (t[item[1]] + t[item[2]]);
            }

            for (std::size_t i = 0, j = mMaxSample; i <= mMaxSample; ++i, --j)
            {
                c.axis[1] = t[j] * c.M[0] + t[i] * c.M[1];
                ComputeVolume(c);
                if (c.volume < mvc.volume)
                {
                    mvc = c;
                }
            }
        }

        virtual void MinimizerConstantT(Candidate& c, Candidate& mvc)
        {
            Number const half = static_cast<Number>(0.5);
            std::vector<Number> s(mMaxSample + 1);
            s[0] = static_cast<Number>(0);
            s[mMaxSample] = static_cast<Number>(1);
            for (auto const& item : mDomainIndex)
            {
                s[item[0]] = half * (s[item[1]] + s[item[2]]);
            }

            for (std::size_t i = 0, j = mMaxSample; i <= mMaxSample; ++i, --j)
            {
                c.axis[0] = s[j] * c.N[0] + s[i] * c.N[1];
                ComputeVolume(c);
                if (c.volume < mvc.volume)
                {
                    mvc = c;
                }
            }
        }

        virtual void MinimizerVariableS(
            Number const& sminNumer, Number const& smaxNumer, Number const& sDenom,
            Candidate& c, Candidate& mvc)
        {
            Number const half = static_cast<Number>(0.5);
            std::vector<Number> s(mMaxSample + 1), oms(mMaxSample + 1);
            s[0] = sminNumer;
            oms[0] = sDenom - sminNumer;
            s[mMaxSample] = smaxNumer;
            oms[mMaxSample] = sDenom - smaxNumer;
            for (auto const& item : mDomainIndex)
            {
                s[item[0]] = half * (s[item[1]] + s[item[2]]);
                oms[item[0]] = half * (oms[item[1]] + oms[item[2]]);
            }

            for (std::size_t i = 0; i <= mMaxSample; ++i)
            {
                c.axis[0] = oms[i] * c.N[0] + s[i] * c.N[1];

                Number q0 = oms[i] * c.f00 + s[i] * c.f10;
                Number q1 = oms[i] * c.f01 + s[i] * c.f11;
                if (q0 > q1)
                {
                    c.axis[1] = q0 * c.M[1] - q1 * c.M[0];
                }
                else
                {
                    c.axis[1] = q1 * c.M[0] - q0 * c.M[1];
                }

                ComputeVolume(c);
                if (c.volume < mvc.volume)
                {
                    mvc = c;
                }
            }
        }

        virtual void MinimizerVariableT(
            T const& tminNumer, T const& tmaxNumer, T const& tDenom,
            Candidate& c, Candidate& mvc)
        {
            Number const half = static_cast<Number>(0.5);
            std::vector<Number> t(mMaxSample + 1), omt(mMaxSample + 1);
            t[0] = tminNumer;
            omt[0] = tDenom - tminNumer;
            t[mMaxSample] = tmaxNumer;
            omt[mMaxSample] = tDenom - tmaxNumer;
            for (auto const& item : mDomainIndex)
            {
                t[item[0]] = half * (t[item[1]] + t[item[2]]);
                omt[item[0]] = half * (omt[item[1]] + omt[item[2]]);
            }

            for (std::size_t i = 0; i <= mMaxSample; ++i)
            {
                Number p0 = omt[i] * c.f00 + t[i] * c.f01;
                Number p1 = omt[i] * c.f10 + t[i] * c.f11;
                if (p0 > p1)
                {
                    c.axis[0] = p0 * c.N[1] - p1 * c.N[0];
                }
                else
                {
                    c.axis[0] = p1 * c.N[0] - p0 * c.N[1];
                }

                c.axis[1] = omt[i] * c.M[0] + t[i] * c.M[1];

                ComputeVolume(c);
                if (c.volume < mvc.volume)
                {
                    mvc = c;
                }
            }
        }

        void Z00Z10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x00
            // 0 0
            // 0 0
            //
            // This case occurs when each edge is shared by two coplanar
            // faces, so we have only two different normals. The normals
            // are perpendicular.

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void P00Z10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x01
            // 0 0
            // + 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void M00Z10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x02
            // 0 0
            // - 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void Z00P10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x04
            // 0 0
            // 0 +

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void P00P10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x05
            // 0 0
            // + +

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void M00P10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x06
            // 0 0
            // - +

            // tmin = 0, tmax = 1
            // s = -f00 / (f10 - f00), (+)/(+)
            // 1-s = f10 / (f10 - f00), (+)/(+)
            // N = (1-s) * N0 + s * N1, omit denominator
            c.axis[0] = c.f10 * c.N[0] - c.f00 * c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void Z00M10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x08
            // 0 0
            // 0 -

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void P00M10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x09
            // 0 0
            // + -

            // tmin = 0, tmax = 1
            // s = f00 / (f00 - f10), (+)/(+)
            // 1-s = -f10 / (f00 - f10), (+)/(+)
            // N = s * N1 + (1-s) * N0, omit denominator
            c.axis[0] = c.f00 * c.N[1] - c.f10 * c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 0, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void M00M10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x0a
            // 0 0
            // - -

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void Z00Z10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x10
            // + 0
            // 0 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x11
            // + 0
            // + 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);
        }

        void M00Z10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x12
            // + 0
            // - 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1
            // t = -f00 / (f01 - f00), (+)/(+)
            // 1-t = f01 / (f01 - f00), (+)/(+)
            // M = (1-t) * M0 + t * M1, omit denominator
            c.axis[1] = c.f01 * c.M[0] - c.f00 * c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void Z00P10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x14
            // + 0
            // 0 +
            // It is not possible for a level curve to connect the corners.

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void P00P10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x15
            // + 0
            // + +

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void M00P10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x16
            // + 0
            // - +

            // smin = 0
            // smax = -f00 / (f10 - f00), (+)/(+)
            Number const zero = static_cast<Number>(0);
            Number f10mf00 = c.f10 - c.f00;
            MinimizerVariableS(zero, -c.f00, f10mf00, c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void Z00M10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x18
            // + 0
            // 0 -

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void P00M10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x19
            // + 0
            // + -

            // tmin = 0, tmax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableT(zero, one, one, c, mvc);
        }

        void M00M10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x1a
            // + 0
            // - -

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void Z00Z10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x20
            // - 0
            // 0 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x21
            // - 0
            // + 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1
            // t = f00 / (f00 - f01), (+)/(+)
            // 1-t = -f01 / (f00 - f01), (+)/(+)
            // M = t * M1 + (1-t) * M0, omit denominator
            c.axis[1] = c.f00 * c.M[1] - c.f01 * c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void M00Z10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x22
            // - 0
            // - 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);
        }

        void Z00P10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x24
            // - 0
            // 0 +

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void P00P10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x25
            // - 0
            // + +

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void M00P10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x26
            // - 0
            // - +

            // tmin = 0, tmax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableT(zero, one, one, c, mvc);
        }

        void Z00M10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x28
            // - 0
            // 0 -
            // It is not possible for a level curve to connect the corners.

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void P00M10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x29
            // - 0
            // + -

            // smin = 0
            // smax = f00 / (f00 - f10), (+)/(+)
            Number const zero = static_cast<Number>(0);
            Number f00mf10 = c.f00 - c.f10;
            MinimizerVariableS(zero, c.f00, f00mf10, c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void M00M10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x2a
            // - 0
            // - -

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void Z00Z10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x40
            // 0 +
            // 0 0

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);

            // smimn = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x41
            // 0 +
            // + 0
            // It is not possible for a level curve to connect the corners.

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[1];
            Pair(c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void M00Z10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x42
            // 0 +
            // - 0

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void Z00P10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x44
            // 0 +
            // 0 +

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);
        }

        void P00P10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x45
            // 0 +
            // + +

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void M00P10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x46
            // 0 +
            // - +

            // tmin = 0, tmax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableT(zero, one, one, c, mvc);
        }

        void Z00M10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x48
            // 0 +
            // 0 -

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1
            // t = -f10 / (f11 - f10), (+)/(+)
            // 1-t = f11 / (f11 - f10), (+)/(+)
            // M = (1-t) * M0 + t * M1, omit denominator
            c.axis[1] = c.f11 * c.M[0] - c.f10 * c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void P00M10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x49
            // 0 +
            // + -

            // smin = f00 / (f00 - f10), (+)/(+)
            // smax = 1
            Number f00mf10 = c.f00 - c.f10;
            MinimizerVariableS(c.f00, f00mf10, f00mf10, c, mvc);

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void M00M10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x4a
            // 0 +
            // - -

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void Z00Z10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x50
            // + +
            // 0 0

            // smin = 0, smax = 1, t = 0s
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x51
            // + +
            // + 0

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void M00Z10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x52
            // + +
            // - 0

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void Z00P10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x54
            // + +
            // 0 +

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void P00P10P01P11(Candidate&, Candidate&)
        {
            // index = 0x55
            // + +
            // + +
            // Nothing to do.
        }

        void M00P10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x56
            // + +
            // - +

            // smin = 0
            // smax = -f00 / (f10 - f00), (+)/(+)
            Number const zero = static_cast<Number>(0);
            Number f10mf00 = c.f10 - c.f00;
            MinimizerVariableS(zero, -c.f00, f10mf00, c, mvc);
        }

        void Z00M10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x58
            // + +
            // 0 -

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void P00M10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x59
            // + +
            // + -

            // smin = f00 / (f00 - f10), (+)/(+)
            // smax = 1
            Number f00mf10 = c.f00 - c.f10;
            MinimizerVariableS(c.f00, f00mf10, f00mf10, c, mvc);
        }

        void M00M10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x5a
            // + +
            // - -

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void Z00Z10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x60
            // - +
            // 0 0

            // tmin = 0, tmax = 1
            // s = -f01 / (f11 - f01), (+)/(+)
            // 1-s = f11 / (f11 - f01), (+)/(+)
            // N = (1-s) * N0 + s * N1, omit denominator
            c.axis[0] = c.f11 * c.N[0] - c.f01 * c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x61
            // - +
            // + 0

            // smin = 0
            // smax = -f01 / (f11 - f01), (+)/(+)
            Number const zero = static_cast<Number>(0);
            Number f11mf01 = c.f11 - c.f01;
            MinimizerVariableS(zero, -c.f01, f11mf01, c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void M00Z10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x62
            // - +
            // - 0

            // tmin = 0, tmax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableT(zero, one, one, c, mvc);
        }

        void Z00P10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x64
            // - +
            // 0 +

            // tmin = 0, tmax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableT(zero, one, one, c, mvc);
        }

        void P00P10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x65
            // - +
            // + +

            // smin = 0
            // smax = -f01 / (f11 - f01), (+)/(+)
            Number const zero = static_cast<Number>(0);
            Number f11mf01 = c.f11 - c.f01;
            MinimizerVariableS(zero, -c.f01, f11mf01, c, mvc);
        }

        void M00P10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x66
            // - +
            // - +

            // tmin = 0, tmax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableT(zero, one, one, c, mvc);
        }

        void Z00M10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x68
            // - +
            // 0 -

            // smin = -f01 / (f11 - f01), (+)/(+)
            // smax = 1
            Number f11mf01 = c.f11 - c.f01;
            MinimizerVariableS(-c.f01, f11mf01, f11mf01, c, mvc);

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void P00M10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x69
            // - +
            // + -
            //
            // The level set F = 0 has two hyperbolic curves, each formed by a
            // pair of endpoints in {(0,t0), (s0,0), (s1,1), (1,t1)}, where
            // s0 = -f00 / (f10 - f00), s1 = -f01 / (f11 - f01),
            // t0 = -f00 / (f01 - f00), t1 = -f10 / (f11 - f10), all quantites
            // in (0,1). The two curves are on opposite sides of the
            // asymptotes
            //   sa = (f01 - f00) / ((f01 - f00) + (f10 - f11))
            //   ta = (f10 - f00) / ((f10 - f00) + (f01 - f11))
            // If s0 < sa, one curve has endpoints {(0,t0),(s0,0)} and the
            // other curve has endpoints {(s1,1),(1,t1)}. If s0 > sa, one
            // curve has endpoints {(0,t0),(s1,1)} and the other curve has
            // endpoints {(s0,0),(1,t1)}. If s0 = sa, then segments of the
            // asymptotes are the two curves for the level set. Define
            // d = f00 * f11 - f10 * f01. It can be shown that
            //   s0 - sa = d / ((f10 - f00)((f10 - f00) + (f01 - f11))
            // The denominator is positive, so sign(s0 - sa) = sign(d). A
            // similar argument applies for the comparison between t0 and ta.

            Number const zero = static_cast<Number>(0);
            Number d = c.f00 * c.f11 - c.f10 * c.f01;
            if (d > zero)
            {
                // endpoints (s0,0) and (1,t1)
                // smin = f00 / (f00 - f10), (+)/(+)
                // smax = 1
                Number f00mf10 = c.f00 - c.f10;
                MinimizerVariableS(c.f00, f00mf10, f00mf10, c, mvc);

                // endpoints (0,t0) and (s1,1)
                // smin = 0
                // smax = -f01 / (f11 - f01), (+)/(+)
                Number f11mf01 = c.f11 - c.f01;
                MinimizerVariableS(zero, -c.f01, f11mf01, c, mvc);
            }
            else if (d < zero)
            {
                // endpoints (0,t0) and (s0,0)
                // smin = 0
                // smax = f00 / (f00 - f10), (+)/(+)
                Number f00mf10 = c.f00 - c.f10;
                MinimizerVariableS(zero, c.f00, f00mf10, c, mvc);

                // endpoints (s1,1) and (1,t1)
                // smin = -f01 / (f11 - f01), (+)/(+)
                // smax = 1
                Number f11mf01 = c.f11 - c.f01;
                MinimizerVariableS(-c.f01, f11mf01, f11mf01, c, mvc);
            }
            else
            {
                // endpoints (sa,0) and (sa,1)
                // sa = (f00 - f01) / ((f00 - f01) + (f11 - f10)), (+)/(+)
                // 1-sa = (f11 - f10) / ((f00 - f01) + (f11 - f10)), (+)/(+)
                // N = (1-sa) * N0 + sa * N1, omit the denominator
                c.axis[0] = (c.f11 - c.f10) * c.N[0] + (c.f00 - c.f01) * c.N[1];
                MinimizerConstantS(c, mvc);

                // endpoints (0,ta) and (1,ta)
                // ta = (f00 - f10) / ((f00 - f10) + (f11 - f01)), (+)/(+)
                // 1-ta = (f11 - f01) / ((f00 - f01) + (f11 - f01)), (+)/(+)
                // M = (1-ta) * M0 + ta * M1, omit the denominator
                c.axis[1] = (c.f11 - c.f01) * c.M[0] + (c.f00 - c.f10) * c.M[1];
                MinimizerConstantT(c, mvc);
            }
        }

        void M00M10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x6a
            // - +
            // - -

            // smin = -f01 / (f11 - f01), (+)/(+)
            // smax = 1
            Number f11mf01 = c.f11 - c.f01;
            MinimizerVariableS(-c.f01, f11mf01, f11mf01, c, mvc);
        }

        void Z00Z10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x80
            // 0 -
            // 0 0

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x81
            // 0 -
            // + 0

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void M00Z10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x82
            // 0 -
            // - 0
            // It is not possible for a level curve to connect the corners.

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[1];
            Pair(c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void Z00P10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x84
            // 0 -
            // 0 +

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1
            // t = f10 / (f10 - f11), (+)/(+)
            // 1-t = -f11 / (f10 - f11), (+)/(+)
            // M = t * M1 + (1-t) * M0, omit the denominator
            c.axis[1] = c.f10 * c.M[1] - c.f11 * c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00P10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x85
            // 0 -
            // + +

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void M00P10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x86
            // 0 -
            // - +

            // smin = -f00 / (f10 - f00), (+)/(+)
            // smax = 1
            Number f10mf00 = c.f10 - c.f00;
            MinimizerVariableS(-c.f00, f10mf00, f10mf00, c, mvc);
        }

        void Z00M10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x88
            // 0 -
            // 0 -

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);
        }

        void P00M10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x89
            // 0 -
            // + -

            // tmin = 0, tmax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableT(zero, one, one, c, mvc);
        }

        void M00M10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x8a
            // 0 -
            // - -

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void Z00Z10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x90
            // + -
            // 0 0

            // tmin = 0, tmax = 1
            // s = f01 / (f01 - f11), (+)/(+)
            // 1-s = -f11 / (f01 - f11), (+)/(+)
            // N = s * N1 + (1-s) * N0, omit the denominator
            c.axis[0] = c.f01 * c.N[1] - c.f11 * c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x91
            // + -
            // + 0

            // tmin = 0, tmax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableT(zero, one, one, c, mvc);
        }

        void M00Z10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x92
            // + -
            // - 0

            // smin = 0
            // smax = f01 / (f01 - f11), (+)/(+)
            Number const zero = static_cast<Number>(0);
            Number f01mf11 = c.f01 - c.f11;
            MinimizerVariableS(zero, c.f01, f01mf11, c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void Z00P10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x94
            // + -
            // 0 +

            // smin = f01 / (f01 - f11), (+)/(+)
            // smax = 1
            Number f01mf11 = c.f01 - c.f11;
            MinimizerVariableS(c.f01, f01mf11, f01mf11, c, mvc);

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void P00P10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x95
            // + -
            // + +

            // smin = f01 / (f01 - f11), (+)/(+)
            // smax = 1
            Number f01mf11 = c.f01 - c.f11;
            MinimizerVariableS(c.f01, f01mf11, f01mf11, c, mvc);
        }

        void M00P10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x96
            // + -
            // - +
            //
            // The level set F = 0 has two hyperbolic curves, each formed by a
            // pair of endpoints in {(0,t0), (s0,0), (s1,1), (1,t1)}, where
            // s0 = -f00 / (f10 - f00), s1 = -f01 / (f11 - f01),
            // t0 = -f00 / (f01 - f00), t1 = -f10 / (f11 - f10), all quantites
            // in (0,1). The two curves are on opposite sides of the
            // asymptotes
            //   sa = (f01 - f00) / ((f01 - f00) + (f10 - f11))
            //   ta = (f10 - f00) / ((f10 - f00) + (f01 - f11))
            // If s0 < sa, one curve has endpoints {(0,t0),(s0,0)} and the
            // other curve has endpoints {(s1,1),(1,t1)}. If s0 > sa, one
            // curve has endpoints {(0,t0),(s1,1)} and the other curve has
            // endpoints {(s0,0),(1,t1)}. If s0 = sa, then segments of the
            // asymptotes are the two curves for the level set. Define
            // d = f00 * f11 - f10 * f01. It can be shown that
            //   s0 - sa = d / ((f10 - f00)((f10 - f00) + (f01 - f11))
            // The denominator is positive, so sign(s0 - sa) = sign(d). A
            // similar argument applies for the comparison between t0 and ta.

            Number const zero = static_cast<Number>(0);
            Number d = c.f00 * c.f11 - c.f10 * c.f01;
            if (d > zero)
            {
                // endpoints (s0,0) and (1,t1)
                // smin = -f00 / (f10 - f00), (+)/(+)
                // smax = 1
                Number f10mf00 = c.f10 - c.f00;
                MinimizerVariableS(-c.f00, f10mf00, f10mf00, c, mvc);

                // endpoints (0,t0) and (s1,1)
                // smin = 0
                // smax = f01 / (f01 - f11)
                Number f01mf11 = c.f01 - c.f11;
                MinimizerVariableS(zero, c.f01, f01mf11, c, mvc);
            }
            else if (d < zero)
            {
                // endpoints (0,t0) and (s0,0)
                // smin = 0
                // smax = -f00 / (f10- f00), (+)/(+)
                Number f10mf00 = c.f10 - c.f00;
                MinimizerVariableS(zero, -c.f00, f10mf00, c, mvc);

                // endpoints (s1,1) and (1,t1)
                // smin = f01 / (f01 - f11), (+)/(+)
                // smax = 1
                Number f01mf11 = c.f01 - c.f11;
                MinimizerVariableS(c.f01, f01mf11, f01mf11, c, mvc);
            }
            else
            {
                // endpoints (sa,0) and (sa,1)
                // sa = (f01 - f00) / ((f01 - f00) + (f10 - f11)), (+)/(+)
                // 1-sa = (f10 - f11) / ((f01 - f00) + (f10 - f11)), (+)/(+)
                // N = (1-sa) * N0 + s1* N1
                c.axis[0] = (c.f10 - c.f11) * c.N[0] + (c.f01 - c.f00) * c.N[1];
                MinimizerConstantS(c, mvc);

                // endpoints (0,ta) and (1,ta)
                // ta = (f10 - f00) / ((f10 - f00) + (f01 - f11)), (+)/(+)
                // 1-ta = (f01 - f11) / ((f10 - f00) + (f01 - f11)), (+)/(+)
                // M = (1-ta) * M0 + ta * M1, omit the denominator
                c.axis[1] = (c.f01 - c.f11) * c.M[0] + (c.f10 - c.f00) * c.M[1];
                MinimizerConstantT(c, mvc);
            }
        }

        void Z00M10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x98
            // + -
            // 0 -

            // tmin = 0, tmax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableT(zero, one, one, c, mvc);
        }

        void P00M10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x99
            // + -
            // + -

            // tmin = 0, tmax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableT(zero, one, one, c, mvc);
        }

        void M00M10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x9a
            // + -
            // - -

            // smin = 0
            // smax = f01 / (f01 - f11), (+)/(+)
            Number const zero = static_cast<Number>(0);
            Number f01mf11 = c.f01 - c.f11;
            MinimizerVariableS(zero, c.f01, f01mf11, c, mvc);
        }

        void Z00Z10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa0
            // - -
            // 0 0

            // smin = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa1
            // - -
            // + 0

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void M00Z10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa2
            // - -
            // - 0

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void Z00P10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa4
            // - -
            // 0 +

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void P00P10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa5
            // - -
            // + +

            // smin = 0, smax = 1
            Number const zero = static_cast<Number>(0);
            Number const one = static_cast<Number>(1);
            MinimizerVariableS(zero, one, one, c, mvc);
        }

        void M00P10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa6
            // - -
            // - +

            // smin = -f00 / (f10 - f00), (+)/(+)
            // smax = 1
            Number f10mf00 = c.f10 - c.f00;
            MinimizerVariableS(-c.f00, f10mf00, f10mf00, c, mvc);
        }

        void Z00M10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa8
            // - -
            // 0 -

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void P00M10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa9
            // - -
            // + -

            // smin = 0
            // smax = f00 / (f00 - f10), (+)/(+)
            Number const zero = static_cast<Number>(0);
            Number f00mf10 = c.f00 - c.f10;
            MinimizerVariableS(zero, c.f00, f00mf10, c, mvc);
        }

        void M00M10M01M11(Candidate&, Candidate&)
        {
            // index = 0xaa
            // - -
            // - -
            // Nothing to do.
        }
    };
}
