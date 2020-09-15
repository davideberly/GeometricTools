// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/ETManifoldMesh.h>
#include <Mathematics/PrimalQuery2.h>
#include <Mathematics/Line.h>
#include <set>

// Delaunay triangulation of points (intrinsic dimensionality 2).
//   VQ = number of vertices
//   V  = array of vertices
//   TQ = number of triangles
//   I  = Array of 3-tuples of indices into V that represent the triangles
//        (3*TQ total elements).  Access via GetIndices(*).
//   A  = Array of 3-tuples of indices into I that represent the adjacent
//        triangles (3*TQ total elements).  Access via GetAdjacencies(*).
// The i-th triangle has vertices
//   vertex[0] = V[I[3*i+0]]
//   vertex[1] = V[I[3*i+1]]
//   vertex[2] = V[I[3*i+2]]
// and edge index pairs
//   edge[0] = <I[3*i+0],I[3*i+1]>
//   edge[1] = <I[3*i+1],I[3*i+2]>
//   edge[2] = <I[3*i+2],I[3*i+0]>
// The triangles adjacent to these edges have indices
//   adjacent[0] = A[3*i+0] is the triangle sharing edge[0]
//   adjacent[1] = A[3*i+1] is the triangle sharing edge[1]
//   adjacent[2] = A[3*i+2] is the triangle sharing edge[2]
// If there is no adjacent triangle, the A[*] value is set to -1.  The
// triangle adjacent to edge[j] has vertices
//   adjvertex[0] = V[I[3*adjacent[j]+0]]
//   adjvertex[1] = V[I[3*adjacent[j]+1]]
//   adjvertex[2] = V[I[3*adjacent[j]+2]]
// The only way to ensure a correct result for the input vertices (assumed to
// be exact) is to choose ComputeType for exact rational arithmetic.  You may
// use BSNumber.  No divisions are performed in this computation, so you do
// not have to use BSRational.
//
// The worst-case choices of N for Real of type BSNumber or BSRational with
// integer storage UIntegerFP32<N> are listed in the next table.  The numerical
// computations are encapsulated in PrimalQuery2<Real>::ToLine and
// PrimalQuery2<Real>::ToCircumcircle, the latter query the dominant one in
// determining N.  We recommend using only BSNumber, because no divisions are
// performed in the convex-hull computations.
//
//    input type | compute type | N
//    -----------+--------------+------
//    float      | BSNumber     |    35
//    double     | BSNumber     |   263
//    float      | BSRational   | 12302
//    double     | BSRational   | 92827

namespace gte
{
    template <typename InputType, typename ComputeType>
    class Delaunay2
    {
    public:
        // The class is a functor to support computing the Delaunay
        // triangulation of multiple data sets using the same class object.
        virtual ~Delaunay2() = default;

        Delaunay2()
            :
            mEpsilon((InputType)0),
            mDimension(0),
            mLine(Vector2<InputType>::Zero(), Vector2<InputType>::Zero()),
            mNumVertices(0),
            mNumUniqueVertices(0),
            mNumTriangles(0),
            mVertices(nullptr),
            mIndex{ { { 0, 1 }, { 1, 2 }, { 2, 0 } } }
        {
        }

        // The input is the array of vertices whose Delaunay triangulation is
        // required.  The epsilon value is used to determine the intrinsic
        // dimensionality of the vertices (d = 0, 1, or 2).  When epsilon is
        // positive, the determination is fuzzy--vertices approximately the
        // same point, approximately on a line, or planar.  The return value
        // is 'true' if and only if the hull construction is successful.
        bool operator()(int numVertices, Vector2<InputType> const* vertices, InputType epsilon)
        {
            mEpsilon = std::max(epsilon, (InputType)0);
            mDimension = 0;
            mLine.origin = Vector2<InputType>::Zero();
            mLine.direction = Vector2<InputType>::Zero();
            mNumVertices = numVertices;
            mNumUniqueVertices = 0;
            mNumTriangles = 0;
            mVertices = vertices;
            mGraph.Clear();
            mIndices.clear();
            mAdjacencies.clear();
            mDuplicates.resize(std::max(numVertices, 3));

            int i, j;
            if (mNumVertices < 3)
            {
                // Delaunay2 should be called with at least three points.
                return false;
            }

            IntrinsicsVector2<InputType> info(mNumVertices, vertices, mEpsilon);
            if (info.dimension == 0)
            {
                // mDimension is 0; mGraph, mIndices, and mAdjacencies are empty
                return false;
            }

            if (info.dimension == 1)
            {
                // The set is (nearly) collinear.
                mDimension = 1;
                mLine = Line2<InputType>(info.origin, info.direction[0]);
                return false;
            }

            mDimension = 2;

            // Compute the vertices for the queries.
            mComputeVertices.resize(mNumVertices);
            mQuery.Set(mNumVertices, &mComputeVertices[0]);
            for (i = 0; i < mNumVertices; ++i)
            {
                for (j = 0; j < 2; ++j)
                {
                    mComputeVertices[i][j] = vertices[i][j];
                }
            }

            // Insert the (nondegenerate) triangle constructed by the call to
            // GetInformation.  This is necessary for the circumcircle-visibility
            // algorithm to work correctly.
            if (!info.extremeCCW)
            {
                std::swap(info.extreme[1], info.extreme[2]);
            }
            if (!mGraph.Insert(info.extreme[0], info.extreme[1], info.extreme[2]))
            {
                return false;
            }

            // Incrementally update the triangulation.  The set of processed
            // points is maintained to eliminate duplicates, either in the
            // original input points or in the points obtained by snap rounding.
            std::set<ProcessedVertex> processed;
            for (i = 0; i < 3; ++i)
            {
                j = info.extreme[i];
                processed.insert(ProcessedVertex(vertices[j], j));
                mDuplicates[j] = j;
            }
            for (i = 0; i < mNumVertices; ++i)
            {
                ProcessedVertex v(vertices[i], i);
                auto iter = processed.find(v);
                if (iter == processed.end())
                {
                    if (!Update(i))
                    {
                        // A failure can occur if ComputeType is not an exact
                        // arithmetic type.
                        return false;
                    }
                    processed.insert(v);
                    mDuplicates[i] = i;
                }
                else
                {
                    mDuplicates[i] = iter->location;
                }
            }
            mNumUniqueVertices = static_cast<int>(processed.size());

            // Assign integer values to the triangles for use by the caller.
            std::map<std::shared_ptr<Triangle>, int> permute;
            i = -1;
            permute[nullptr] = i++;
            for (auto const& element : mGraph.GetTriangles())
            {
                permute[element.second] = i++;
            }

            // Put Delaunay triangles into an array (vertices and adjacency info).
            mNumTriangles = static_cast<int>(mGraph.GetTriangles().size());
            int numindices = 3 * mNumTriangles;
            if (numindices > 0)
            {
                mIndices.resize(numindices);
                mAdjacencies.resize(numindices);
                i = 0;
                for (auto const& element : mGraph.GetTriangles())
                {
                    std::shared_ptr<Triangle> tri = element.second;
                    for (j = 0; j < 3; ++j, ++i)
                    {
                        mIndices[i] = tri->V[j];
                        mAdjacencies[i] = permute[tri->T[j].lock()];
                    }
                }
            }

            return true;
        }

        // Dimensional information.  If GetDimension() returns 1, the points
        // lie on a line P+t*D (fuzzy comparison when epsilon > 0).  You can
        // sort these if you need a polyline output by projecting onto the
        // line each vertex X = P+t*D, where t = Dot(D,X-P).
        inline InputType GetEpsilon() const
        {
            return mEpsilon;
        }

        inline int GetDimension() const
        {
            return mDimension;
        }

        inline Line2<InputType> const& GetLine() const
        {
            return mLine;
        }

        // Member access.
        inline int GetNumVertices() const
        {
            return mNumVertices;
        }

        inline int GetNumUniqueVertices() const
        {
            return mNumUniqueVertices;
        }

        inline int GetNumTriangles() const
        {
            return mNumTriangles;
        }

        inline Vector2<InputType> const* GetVertices() const
        {
            return mVertices;
        }

        inline PrimalQuery2<ComputeType> const& GetQuery() const
        {
            return mQuery;
        }

        inline ETManifoldMesh const& GetGraph() const
        {
            return mGraph;
        }

        inline std::vector<int> const& GetIndices() const
        {
            return mIndices;
        }

        inline std::vector<int> const& GetAdjacencies() const
        {
            return mAdjacencies;
        }

        // If 'vertices' has no duplicates, GetDuplicates()[i] = i for all i.
        // If vertices[i] is the first occurrence of a vertex and if
        // vertices[j] is found later, then GetDuplicates()[j] = i.
        inline std::vector<int> const& GetDuplicates() const
        {
            return mDuplicates;
        }

        // Locate those triangle edges that do not share other triangles.  The
        // returned array has hull.size() = 2*numEdges, each pair representing
        // an edge.  The edges are not ordered, but the pair of vertices for
        // an edge is ordered so that they conform to a counterclockwise
        // traversal of the hull.  The return value is 'true' if and only if
        // the dimension is 2.
        bool GetHull(std::vector<int>& hull) const
        {
            if (mDimension == 2)
            {
                // Count the number of edges that are not shared by two
                // triangles.
                int numEdges = 0;
                for (auto adj : mAdjacencies)
                {
                    if (adj == -1)
                    {
                        ++numEdges;
                    }
                }

                if (numEdges > 0)
                {
                    // Enumerate the edges.
                    hull.resize(2 * numEdges);
                    int current = 0, i = 0;
                    for (auto adj : mAdjacencies)
                    {
                        if (adj == -1)
                        {
                            int tri = i / 3, j = i % 3;
                            hull[current++] = mIndices[3 * tri + j];
                            hull[current++] = mIndices[3 * tri + ((j + 1) % 3)];
                        }
                        ++i;
                    }
                    return true;
                }
                else
                {
                    LogError("Unexpected. There must be at least one triangle.");
                }
            }
            else
            {
                LogError("The dimension must be 2.");
            }
        }

        // Get the vertex indices for triangle i.  The function returns 'true'
        // when the dimension is 2 and i is a valid triangle index, in which
        // case the vertices are valid; otherwise, the function returns
        // 'false' and the vertices are invalid.
        bool GetIndices(int i, std::array<int, 3>& indices) const
        {
            if (mDimension == 2)
            {
                int numTriangles = static_cast<int>(mIndices.size() / 3);
                if (0 <= i && i < numTriangles)
                {
                    indices[0] = mIndices[3 * i];
                    indices[1] = mIndices[3 * i + 1];
                    indices[2] = mIndices[3 * i + 2];
                    return true;
                }
            }
            else
            {
                LogError("The dimension must be 2.");
            }
            return false;
        }

        // Get the indices for triangles adjacent to triangle i.  The function
        // returns 'true' when the dimension is 2 and if i is a valid triangle
        // index, in which case the adjacencies are valid; otherwise, the
        // function returns 'false' and the adjacencies are invalid.
        bool GetAdjacencies(int i, std::array<int, 3>& adjacencies) const
        {
            if (mDimension == 2)
            {
                int numTriangles = static_cast<int>(mIndices.size() / 3);
                if (0 <= i && i < numTriangles)
                {
                    adjacencies[0] = mAdjacencies[3 * i];
                    adjacencies[1] = mAdjacencies[3 * i + 1];
                    adjacencies[2] = mAdjacencies[3 * i + 2];
                    return true;
                }
            }
            else
            {
                LogError("The dimension must be 2.");
            }
            return false;
        }

        // Support for searching the triangulation for a triangle that
        // contains a point.  If there is a containing triangle, the returned
        // value is a triangle index i with 0 <= i < GetNumTriangles().  If
        // there is not a containing triangle, -1 is returned.  The
        // computations are performed using exact rational arithmetic.
        //
        // The SearchInfo input stores information about the triangle search
        // when looking for the triangle (if any) that contains p.  The first
        // triangle searched is 'initialTriangle'.  On return 'path' stores
        // those (ordered) triangle indices visited during the search.  The
        // last visited triangle has index 'finalTriangle and vertex indices
        // 'finalV[0,1,2]', stored in counterclockwise order.  The last edge
        // of the search is <finalV[0],finalV[1]>.  For spatially coherent
        // inputs p for numerous calls to this function, you will want to
        // specify 'finalTriangle' from the previous call as 'initialTriangle'
        // for the next call, which should reduce search times.
        struct SearchInfo
        {
            int initialTriangle;
            int numPath;
            std::vector<int> path;
            int finalTriangle;
            std::array<int, 3> finalV;
        };

        int GetContainingTriangle(Vector2<InputType> const& p, SearchInfo& info) const
        {
            if (mDimension == 2)
            {
                Vector2<ComputeType> test{ p[0], p[1] };

                int numTriangles = static_cast<int>(mIndices.size() / 3);
                info.path.resize(numTriangles);
                info.numPath = 0;
                int triangle;
                if (0 <= info.initialTriangle && info.initialTriangle < numTriangles)
                {
                    triangle = info.initialTriangle;
                }
                else
                {
                    info.initialTriangle = 0;
                    triangle = 0;
                }

                // Use triangle edges as binary separating lines.
                for (int i = 0; i < numTriangles; ++i)
                {
                    int ibase = 3 * triangle;
                    int const* v = &mIndices[ibase];

                    info.path[info.numPath++] = triangle;
                    info.finalTriangle = triangle;
                    info.finalV[0] = v[0];
                    info.finalV[1] = v[1];
                    info.finalV[2] = v[2];

                    if (mQuery.ToLine(test, v[0], v[1]) > 0)
                    {
                        triangle = mAdjacencies[ibase];
                        if (triangle == -1)
                        {
                            info.finalV[0] = v[0];
                            info.finalV[1] = v[1];
                            info.finalV[2] = v[2];
                            return -1;
                        }
                        continue;
                    }

                    if (mQuery.ToLine(test, v[1], v[2]) > 0)
                    {
                        triangle = mAdjacencies[ibase + 1];
                        if (triangle == -1)
                        {
                            info.finalV[0] = v[1];
                            info.finalV[1] = v[2];
                            info.finalV[2] = v[0];
                            return -1;
                        }
                        continue;
                    }

                    if (mQuery.ToLine(test, v[2], v[0]) > 0)
                    {
                        triangle = mAdjacencies[ibase + 2];
                        if (triangle == -1)
                        {
                            info.finalV[0] = v[2];
                            info.finalV[1] = v[0];
                            info.finalV[2] = v[1];
                            return -1;
                        }
                        continue;
                    }

                    return triangle;
                }
            }
            else
            {
                LogError("The dimension must be 2.");
            }
            return -1;
        }

    protected:
        // Support for incremental Delaunay triangulation.
        typedef ETManifoldMesh::Triangle Triangle;

        bool GetContainingTriangle(int i, std::shared_ptr<Triangle>& tri) const
        {
            int numTriangles = static_cast<int>(mGraph.GetTriangles().size());
            for (int t = 0; t < numTriangles; ++t)
            {
                int j;
                for (j = 0; j < 3; ++j)
                {
                    int v0 = tri->V[mIndex[j][0]];
                    int v1 = tri->V[mIndex[j][1]];
                    if (mQuery.ToLine(i, v0, v1) > 0)
                    {
                        // Point i sees edge <v0,v1> from outside the triangle.
                        auto adjTri = tri->T[j].lock();
                        if (adjTri)
                        {
                            // Traverse to the triangle sharing the face.
                            tri = adjTri;
                            break;
                        }
                        else
                        {
                            // We reached a hull edge, so the point is outside
                            // the hull.  TODO:  Once a hull data structure is
                            // in place, return tri->T[j] as the candidate for
                            // starting a search for visible hull edges.
                            return false;
                        }
                    }

                }

                if (j == 3)
                {
                    // The point is inside all four edges, so the point is inside
                    // a triangle.
                    return true;
                }
            }

            LogError("Unexpected termination of loop.");
        }

        bool GetAndRemoveInsertionPolygon(int i, std::set<std::shared_ptr<Triangle>>& candidates,
            std::set<EdgeKey<true>>& boundary)
        {
            // Locate the triangles that make up the insertion polygon.
            ETManifoldMesh polygon;
            while (candidates.size() > 0)
            {
                std::shared_ptr<Triangle> tri = *candidates.begin();
                candidates.erase(candidates.begin());

                for (int j = 0; j < 3; ++j)
                {
                    auto adj = tri->T[j].lock();
                    if (adj && candidates.find(adj) == candidates.end())
                    {
                        int a0 = adj->V[0];
                        int a1 = adj->V[1];
                        int a2 = adj->V[2];
                        if (mQuery.ToCircumcircle(i, a0, a1, a2) <= 0)
                        {
                            // Point i is in the circumcircle.
                            candidates.insert(adj);
                        }
                    }
                }

                if (!polygon.Insert(tri->V[0], tri->V[1], tri->V[2]))
                {
                    return false;
                }
                if (!mGraph.Remove(tri->V[0], tri->V[1], tri->V[2]))
                {
                    return false;
                }
            }

            // Get the boundary edges of the insertion polygon.
            for (auto const& element : polygon.GetTriangles())
            {
                std::shared_ptr<Triangle> tri = element.second;
                for (int j = 0; j < 3; ++j)
                {
                    if (!tri->T[j].lock())
                    {
                        boundary.insert(EdgeKey<true>(tri->V[mIndex[j][0]], tri->V[mIndex[j][1]]));
                    }
                }
            }
            return true;
        }

        bool Update(int i)
        {
            // The return value of mGraph.Insert(...) is nullptr if there was
            // a failure to insert.  The Update function will return 'false'
            // when the insertion fails.

            auto const& tmap = mGraph.GetTriangles();
            std::shared_ptr<Triangle> tri = tmap.begin()->second;
            if (GetContainingTriangle(i, tri))
            {
                // The point is inside the convex hull.  The insertion polygon
                // contains only triangles in the current triangulation; the
                // hull does not change.

                // Use a depth-first search for those triangles whose
                // circumcircles contain point i.
                std::set<std::shared_ptr<Triangle>> candidates;
                candidates.insert(tri);

                // Get the boundary of the insertion polygon C that contains
                // the triangles whose circumcircles contain point i.  Polygon
                // C contains the point i.
                std::set<EdgeKey<true>> boundary;
                if (!GetAndRemoveInsertionPolygon(i, candidates, boundary))
                {
                    return false;
                }

                // The insertion polygon consists of the triangles formed by
                // point i and the faces of C.
                for (auto const& key : boundary)
                {
                    int v0 = key.V[0];
                    int v1 = key.V[1];
                    if (mQuery.ToLine(i, v0, v1) < 0)
                    {
                        if (!mGraph.Insert(i, v0, v1))
                        {
                            return false;
                        }
                    }
                    // else:  Point i is on an edge of 'tri', so the
                    // subdivision has degenerate triangles.  Ignore these.
                }
            }
            else
            {
                // The point is outside the convex hull.  The insertion
                // polygon is formed by point i and any triangles in the
                // current triangulation whose circumcircles contain point i.

                // Locate the convex hull of the triangles.  TODO:  Maintain a
                // hull data structure that is updated incrementally.
                std::set<EdgeKey<true>> hull;
                for (auto const& element : tmap)
                {
                    std::shared_ptr<Triangle> t = element.second;
                    for (int j = 0; j < 3; ++j)
                    {
                        if (!t->T[j].lock())
                        {
                            hull.insert(EdgeKey<true>(t->V[mIndex[j][0]], t->V[mIndex[j][1]]));
                        }
                    }
                }

                // TODO:  Until the hull change, for now just iterate over all
                // the hull edges and use the ones visible to point i to
                // locate the insertion polygon.
                auto const& emap = mGraph.GetEdges();
                std::set<std::shared_ptr<Triangle>> candidates;
                std::set<EdgeKey<true>> visible;
                for (auto const& key : hull)
                {
                    int v0 = key.V[0];
                    int v1 = key.V[1];
                    if (mQuery.ToLine(i, v0, v1) > 0)
                    {
                        auto iter = emap.find(EdgeKey<false>(v0, v1));
                        if (iter != emap.end() && iter->second->T[1].lock() == nullptr)
                        {
                            auto adj = iter->second->T[0].lock();
                            if (adj && candidates.find(adj) == candidates.end())
                            {
                                int a0 = adj->V[0];
                                int a1 = adj->V[1];
                                int a2 = adj->V[2];
                                if (mQuery.ToCircumcircle(i, a0, a1, a2) <= 0)
                                {
                                    // Point i is in the circumcircle.
                                    candidates.insert(adj);
                                }
                                else
                                {
                                    // Point i is not in the circumcircle but
                                    // the hull edge is visible.
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

                // Get the boundary of the insertion subpolygon C that
                // contains the triangles whose circumcircles contain point i.
                std::set<EdgeKey<true>> boundary;
                if (!GetAndRemoveInsertionPolygon(i, candidates, boundary))
                {
                    return false;
                }

                // The insertion polygon P consists of the triangles formed by
                // point i and the back edges of C *and* the visible edges of
                // mGraph-C.
                for (auto const& key : boundary)
                {
                    int v0 = key.V[0];
                    int v1 = key.V[1];
                    if (mQuery.ToLine(i, v0, v1) < 0)
                    {
                        // This is a back edge of the boundary.
                        if (!mGraph.Insert(i, v0, v1))
                        {
                            return false;
                        }
                    }
                }
                for (auto const& key : visible)
                {
                    if (!mGraph.Insert(i, key.V[1], key.V[0]))
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        // The epsilon value is used for fuzzy determination of intrinsic
        // dimensionality.  If the dimension is 0 or 1, the constructor
        // returns early.  The caller is responsible for retrieving the
        // dimension and taking an alternate path should the dimension be
        // smaller than 2.  If the dimension is 0, the caller may as well
        // treat all vertices[] as a single point, say, vertices[0].  If the
        // dimension is 1, the caller can query for the approximating line and
        // project vertices[] onto it for further processing.
        InputType mEpsilon;
        int mDimension;
        Line2<InputType> mLine;

        // The array of vertices used for geometric queries.  If you want to
        // be certain of a correct result, choose ComputeType to be BSNumber.
        std::vector<Vector2<ComputeType>> mComputeVertices;
        PrimalQuery2<ComputeType> mQuery;

        // The graph information.
        int mNumVertices;
        int mNumUniqueVertices;
        int mNumTriangles;
        Vector2<InputType> const* mVertices;
        ETManifoldMesh mGraph;
        std::vector<int> mIndices;
        std::vector<int> mAdjacencies;

        // If a vertex occurs multiple times in the 'vertices' input to the
        // constructor, the first processed occurrence of that vertex has an
        // index stored in this array.  If there are no duplicates, then
        // mDuplicates[i] = i for all i.

        struct ProcessedVertex
        {
            ProcessedVertex() = default;

            ProcessedVertex(Vector2<InputType> const& inVertex, int inLocation)
                :
                vertex(inVertex),
                location(inLocation)
            {
            }

            bool operator<(ProcessedVertex const& v) const
            {
                return vertex < v.vertex;
            }

            Vector2<InputType> vertex;
            int location;
        };

        std::vector<int> mDuplicates;

        // Indexing for the vertices of the triangle adjacent to a vertex.
        // The edge adjacent to vertex j is <mIndex[j][0], mIndex[j][1]> and
        // is listed so that the triangle interior is to your left as you walk
        // around the edges.  TODO: Use the "opposite edge" to be consistent
        // with that of TetrahedronKey.  The "opposite" idea extends easily to
        // higher dimensions.
        std::array<std::array<int, 2>, 3> mIndex;
    };
}
