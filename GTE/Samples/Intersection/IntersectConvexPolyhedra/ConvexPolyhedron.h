// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.12.14

#pragma once

#include <Mathematics/DistPointHyperplane.h>
#include <Mathematics/Vector2.h>
#include "MTMesh.h"

template <typename Real>
class ConvexPolyhedron : public MTMesh
{
public:
    typedef typename std::vector<gte::Vector2<Real>> V2Array;
    typedef typename std::vector<gte::Vector3<Real>> V3Array;
    typedef typename std::vector<gte::Plane3<Real>> PArray;
    typedef typename std::vector<int32_t> IArray;

    // Construction.
    ConvexPolyhedron()
        :
        mPoints{},
        mPlanes{},
        mCentroid{ static_cast<Real>(0), static_cast<Real>(0), static_cast<Real>(0) },
        mQuery{}
    {
    }

    ConvexPolyhedron(V3Array const& points, IArray const& indices)
    {
        Create(points, indices);
    }

    ConvexPolyhedron(V3Array const& points, IArray const& indices, PArray const& planes)
    {
        Create(points, indices, planes);
    }

    ConvexPolyhedron(ConvexPolyhedron const& other)
        :
        MTMesh(other),
        mPoints(other.mPoints),
        mPlanes(other.mPlanes)
    {
    }

    void Create(V3Array const& points, IArray const& indices)
    {
        LogAssert(points.size() >= 4 && indices.size() >= 4,
            "Polyhedron must be at least a tetrahedron.");

        int32_t const numVertices = static_cast<int32_t>(points.size());
        int32_t const numTriangles = static_cast<int32_t>(indices.size() / 3);
        int32_t const numEdges = numVertices + numTriangles - 2;
        Reset(numVertices, numEdges, numTriangles);
        mPoints = points;

        // Copy polyhedron points into vertex array.  Compute centroid for use
        // in making sure the triangles are counterclockwise oriented when
        // viewed from the outside.
        ComputeCentroid();

        // Get polyhedron edge and triangle information.
        int32_t const* currentIndex = indices.data();
        for (int32_t t = 0; t < numTriangles; ++t)
        {
            // Get vertex indices for triangle.
            int32_t v0 = *currentIndex++;
            int32_t v1 = *currentIndex++;
            int32_t v2 = *currentIndex++;

            // Make sure triangle is counterclockwise.
            gte::Vector3<Real>& vertex0 = mPoints[v0];
            gte::Vector3<Real>& vertex1 = mPoints[v1];
            gte::Vector3<Real>& vertex2 = mPoints[v2];

            gte::Vector3<Real> diff = mCentroid - vertex0;
            gte::Vector3<Real> edge1 = vertex1 - vertex0;
            gte::Vector3<Real> edge2 = vertex2 - vertex0;
            gte::Vector3<Real> normal = Cross(edge1, edge2);
            Real length = Length(normal);
            if (length > (Real)0)
            {
                normal /= length;
            }
            else
            {
                // The triangle is degenerate, use a "normal" that points
                // towards the centroid.
                normal = diff;
                Normalize(normal);
            }

            Real signedDistance = Dot(normal, diff);
            if (signedDistance < (Real)0)
            {
                // The triangle is counterclockwise.
                Insert(v0, v1, v2);
            }
            else
            {
                // The triangle is clockwise.
                Insert(v0, v2, v1);
            }
        }

        UpdatePlanes();
    }

    void Create(const V3Array& points, const IArray& indices, const PArray& planes)
    {
        LogAssert(points.size() >= 4 && indices.size() >= 4,
            "Polyhedron must be at least a tetrahedron.");

        int32_t const numVertices = static_cast<int32_t>(points.size());
        int32_t const numTriangles = static_cast<int32_t>(indices.size() / 3);
        int32_t const numEdges = numVertices + numTriangles - 2;
        Reset(numVertices, numEdges, numTriangles);
        mPoints = points;
        mPlanes = planes;

        // Copy polyhedron points into vertex array.  Compute centroid for use
        // in making sure the triangles are counterclockwise oriented when
        // viewed from the outside.
        ComputeCentroid();

        // Get polyhedron edge and triangle information.
        int32_t const* currentIndex = indices.data();
        for (int32_t t = 0; t < numTriangles; ++t)
        {
            // Get vertex indices for triangle.
            int32_t v0 = *currentIndex++;
            int32_t v1 = *currentIndex++;
            int32_t v2 = *currentIndex++;

            Real signedDistance = mQuery(mCentroid, mPlanes[t]).signedDistance;
            if (signedDistance > (Real)0)
            {
                // The triangle is counterclockwise.
                Insert(v0, v1, v2);
            }
            else
            {
                // The triangle is clockwise.
                Insert(v0, v2, v1);
            }
        }
    }

    // Assignment.
    ConvexPolyhedron& operator=(const ConvexPolyhedron& polyhedron)
    {
        MTMesh::operator=(polyhedron);
        mPoints = polyhedron.mPoints;
        mPlanes = polyhedron.mPlanes;
        return *this;
    }

    // Read points and planes.
    inline V3Array const& GetPoints() const
    {
        return mPoints;
    }

    inline gte::Vector3<Real> const& GetPoint(int32_t i) const
    {
        return mPoints[i];
    }

    inline void SetPoint(int32_t i, gte::Vector3<Real> const& point)
    {
        mPoints[i] = point;
    }

    inline PArray const& GetPlanes() const
    {
        return mPlanes;
    }

    inline gte::Plane3<Real> const& GetPlane(int32_t i) const
    {
        return mPlanes[i];
    }

    // Allow vertex modification.  The caller is responsible for preserving
    // the convexity.  After modifying the vertices, call UpdatePlanes to
    // recompute the planes of the polyhedron faces.
    int32_t AddPoint(gte::Vector3<Real> const& point)
    {
        int32_t numPoints = static_cast<int32_t>(mPoints.size());
        mPoints.push_back(point);
        return numPoints;
    }

    void UpdatePlanes()
    {
        // The planes are constructed to have *inner-pointing* normals.  This
        // supports the Wild Magic software clipping code that was based on a
        // view frustum having inner-pointing normals.
        ComputeCentroid();

        int32_t const numTriangles = mTriangles.GetNumElements();
        mPlanes.resize(numTriangles);
        for (int32_t t = 0; t < numTriangles; ++t)
        {
            MTTriangle& triangle = mTriangles[t];
            int32_t v0 = GetVLabel(triangle.GetVertex(0));
            int32_t v1 = GetVLabel(triangle.GetVertex(1));
            int32_t v2 = GetVLabel(triangle.GetVertex(2));
            gte::Vector3<Real>& vertex0 = mPoints[v0];
            gte::Vector3<Real>& vertex1 = mPoints[v1];
            gte::Vector3<Real>& vertex2 = mPoints[v2];

            gte::Vector3<Real> diff = mCentroid - vertex0;
            gte::Vector3<Real> edge1 = vertex1 - vertex0;
            gte::Vector3<Real> edge2 = vertex2 - vertex0;
            gte::Vector3<Real> normal = Cross(edge2, edge1);
            Real length = Length(normal);
            if (length > (Real)0)
            {
                normal /= length;
                Real dot = Dot(normal, diff);
                if (dot < (Real)0)
                {
                    normal = -normal;
                }
            }
            else
            {
                // The triangle is degenerate, use a "normal" that points
                // towards the centroid.
                normal = diff;
                Normalize(normal);
            }

            // The plane has inner-pointing normal.
            mPlanes[t] = gte::Plane3<Real>(normal, Dot(normal, vertex0));
        }
    }

    // Test for convexity:  Assuming the application has guaranteed that the
    // mesh is manifold and closed, this function will iterate over the faces
    // of the polyhedron and verify for each that the polyhedron vertices are
    // all on the nonnegative side of the plane.  The threshold is the value
    // that the plane distance d is compared to, d < 0.  In theory the
    // distances should all be nonegative.  Floating point round-off errors
    // can cause some small distances, so you might set epsilon to a small
    // negative number.
    bool ValidateHalfSpaceProperty(Real epsilon = (Real)0) const
    {
        Real maxSignedDistance = -std::numeric_limits<Real>::max();
        Real minSignedDistance = std::numeric_limits<Real>::max();
        int32_t const numPoints = static_cast<int32_t>(mPoints.size());
        int32_t const numTriangles = mTriangles.GetNumElements();
        for (int32_t t = 0; t < numTriangles; ++t)
        {
            gte::Plane3<Real> const& plane = mPlanes[t];
            for (int32_t i = 0; i < numPoints; ++i)
            {
                Real signedDistance = mQuery(mPoints[i], plane).signedDistance;
                if (signedDistance < epsilon)
                {
                    return false;
                }
                if (signedDistance < minSignedDistance)
                {
                    minSignedDistance = signedDistance;
                }
                if (signedDistance > maxSignedDistance)
                {
                    maxSignedDistance = signedDistance;
                }
            }
        }

        return true;
    }

    void ComputeCentroid()
    {
        mCentroid = { (Real)0, (Real)0, (Real)0 };
        for (auto const& point : mPoints)
        {
            mCentroid += point;
        }
        mCentroid /= static_cast<Real>(mPoints.size());
    }

    inline gte::Vector3<Real> const& GetCentroid() const
    {
        return mCentroid;
    }

    // Discard the portion of the mesh on the negative side of the plane.
    bool Clip(gte::Plane3<Real> const& plane, ConvexPolyhedron& intersection) const
    {
        Clipper clipper(*this);
        int32_t side = clipper.Clip(plane);

        if (side > 0)
        {
            intersection = *this;
            return true;
        }

        if (side < 0)
        {
            return false;
        }

        clipper.Convert(intersection);
        return true;
    }

    // Compute the polyhedron of intersection.
    bool FindIntersection(const ConvexPolyhedron& polyhedron, ConvexPolyhedron& intersection) const
    {
        Clipper clipper(*this);

        for (auto const& plane : polyhedron.GetPlanes())
        {
            if (clipper.Clip(plane) < 0)
            {
                return false;
            }
        }

        clipper.Convert(intersection);
        return true;
    }

    // Compute all possible intersections of the polyhedra.  The output array
    // is not fully populated; an output polyhedron is invalid when
    // GetPoints() returns an array with no elements.
    static void FindAllIntersections(std::vector<ConvexPolyhedron> const& polyhedra,
        std::vector<ConvexPolyhedron>& intersections)
    {
        // Only 2^16 possible combinations for intersections are currently
        // supported.  If you need more, then GetHighBit(int32_t) must be modified
        // to handle more than 16-bit inputs.
        int32_t const numInPolyhedra = static_cast<int32_t>(polyhedra.size());
        if (numInPolyhedra <= 0 || numInPolyhedra > 16)
        {
            return;
        }

        int32_t numOutPolyhedra = (1 << numInPolyhedra);
        std::vector<bool> needsTesting(numOutPolyhedra);
        intersections.clear();
        intersections.resize(numOutPolyhedra);
        int32_t i, j;
        for (i = 0; i < numOutPolyhedra; ++i)
        {
            needsTesting[i] = true;
        }

        // Trivial cases, zero or one polyhedron--already the intersection.
        needsTesting[0] = false;
        for (i = 0; i < numInPolyhedra; ++i)
        {
            j = (1 << i);
            needsTesting[j] = false;
            intersections[j] = ConvexPolyhedron(polyhedra[i]);
        }

        for (i = 3; i < numOutPolyhedra; ++i)
        {
            if (needsTesting[i])
            {
                // In binary, i = b[m]...b[0] where b[m] is not zero (the
                // high-order bit.  Also, i1 = b[m-1]...b[0] is not zero.  If
                // it were, we would have ruled out the combination by the
                // j-loop below.  Therefore, i0 = b[m]0...0 and i1 correspond
                // to already existing polyhedra.  The intersection finding
                // just needs to look at the intersection of the two
                // polyhedra.
                int32_t i0 = GetHighBit(i);
                int32_t i1 = i & ~i0;
                intersections[i] = FindSolidIntersection(intersections[i0], intersections[i1]);
                if (intersections[i].GetPoints().size() == 0)
                {
                    // No intersection for this combination.  No need to test
                    // other combinations that include this one.
                    for (j = 0; j < numOutPolyhedra; ++j)
                    {
                        if ((i & j) == i)
                        {
                            needsTesting[j] = false;
                        }
                    }
                }
#ifdef _DEBUG
                else  // Test whether we have a well-formed convex polyhedron.
                {
                    gte::Vector3<Real> centroid = intersections[i].GetCentroid();
                    bool contains = intersections[i].ContainsPoint(centroid);
                    LogAssert(contains, "Polyhedron is not well formed.");
                }
#endif
            }
        }
    }

    Real GetSurfaceArea() const
    {
        Real surfaceArea = (Real)0;

        int32_t const numTriangles = mTriangles.GetNumElements();
        for (int32_t t = 0; t < numTriangles; ++t)
        {
            MTTriangle const& triangle = mTriangles[t];
            int32_t v0 = GetVLabel(triangle.GetVertex(0));
            int32_t v1 = GetVLabel(triangle.GetVertex(1));
            int32_t v2 = GetVLabel(triangle.GetVertex(2));
            gte::Vector3<Real> const& vertex0 = mPoints[v0];
            gte::Vector3<Real> const& vertex1 = mPoints[v1];
            gte::Vector3<Real> const& vertex2 = mPoints[v2];
            gte::Vector3<Real> const& normal = mPlanes[t].normal;

            surfaceArea += GetTriangleArea(normal, vertex0, vertex1, vertex2);
        }

        return surfaceArea;
    }

    Real GetVolume() const
    {
        Real volume = (Real)0;

        int32_t const numTriangles = mTriangles.GetNumElements();
        for (int32_t t = 0; t < numTriangles; ++t)
        {
            MTTriangle const& triangle = mTriangles[t];
            int32_t v0 = GetVLabel(triangle.GetVertex(0));
            int32_t v1 = GetVLabel(triangle.GetVertex(1));
            int32_t v2 = GetVLabel(triangle.GetVertex(2));
            gte::Vector3<Real> const& vertex0 = mPoints[v0];
            gte::Vector3<Real> const& vertex1 = mPoints[v1];
            gte::Vector3<Real> const& vertex2 = mPoints[v2];
            volume += Dot(vertex0, Cross(vertex1, vertex2));
        }

        volume /= (Real)6;
        return volume;
    }

    bool ContainsPoint(gte::Vector3<Real> const& point) const
    {
        int32_t const numTriangles = mTriangles.GetNumElements();
        for (int32_t t = 0; t < numTriangles; ++t)
        {
            Real signedDistance = mQuery(point, mPlanes[t]).signedDistance;
            if (signedDistance < (Real)0)
            {
                return false;
            }
        }
        return true;
    }

    // The eye point must be outside the polyhedron.  The output is the
    // terminator, an ordered list of vertices forming a simple closed
    // polyline that separates the visible from invisible faces of the
    // polyhedron.
    void ComputeTerminator(gte::Vector3<Real> const& eye, V3Array& terminator)
    {
        // Temporary storage for signed distances from eye to triangles.
        int32_t const numTriangles = mTriangles.GetNumElements();
        std::vector<Real> distances(numTriangles);
        int32_t i, j;
        for (i = 0; i < numTriangles; ++i)
        {
            distances[i] = std::numeric_limits<Real>::max();
        }

        // Start a search for a front-facing triangle that has an adjacent
        // back-facing triangle or for a back-facing triangle that has an
        // adjacent front-facing triangle.
        int32_t tCurrent = 0;
        MTTriangle* triangle = &mTriangles[tCurrent];
        Real triDistance = GetDistance(eye, tCurrent, distances);
        int32_t eFirst = -1;
        for (i = 0; i < numTriangles; ++i)
        {
            // Check adjacent neighbors for edge of terminator.  Such an edge
            // occurs if the signed distance changes sign.
            int32_t minIndex = -1;
            Real minAbsDistance = std::numeric_limits<Real>::max();
            Real adjDistance[3];
            for (j = 0; j < 3; ++j)
            {
                adjDistance[j] = GetDistance(eye, triangle->GetAdjacent(j), distances);
                if (IsNegativeProduct(triDistance, adjDistance[j]))
                {
                    eFirst = triangle->GetEdge(j);
                    break;
                }

                Real absDistance = std::fabs(adjDistance[j]);
                if (absDistance < minAbsDistance)
                {
                    minAbsDistance = absDistance;
                    minIndex = j;
                }
            }
            if (j < 3)
            {
                break;
            }

            // First edge not found during this iteration.  Move to adjacent
            // triangle whose distance is smallest of all adjacent triangles.
            tCurrent = triangle->GetAdjacent(minIndex);
            triangle = &mTriangles[tCurrent];
            triDistance = adjDistance[minIndex];
        }
        LogAssert(i < numTriangles, "Unexpected condition.");

        MTEdge& edgeFirst = mEdges[eFirst];
        terminator.push_back(mPoints[GetVLabel(edgeFirst.GetVertex(0))]);
        terminator.push_back(mPoints[GetVLabel(edgeFirst.GetVertex(1))]);

        // Walk along the terminator.
        int32_t vFirst = edgeFirst.GetVertex(0);
        int32_t v = edgeFirst.GetVertex(1);
        int32_t e = eFirst;
        int32_t const numEdges = mEdges.GetNumElements();
        for (i = 0; i < numEdges; ++i)
        {
            // Search all edges sharing the vertex for another terminator
            // edge.
            int32_t const jmax = mVertices[v].GetNumEdges();
            for (j = 0; j < jmax; ++j)
            {
                int32_t eNext = mVertices[v].GetEdge(j);
                if (eNext == e)
                {
                    continue;
                }

                Real distance0 = GetDistance(eye, mEdges[eNext].GetTriangle(0), distances);
                Real distance1 = GetDistance(eye, mEdges[eNext].GetTriangle(1), distances);
                if (IsNegativeProduct(distance0, distance1))
                {
                    if (mEdges[eNext].GetVertex(0) == v)
                    {
                        v = mEdges[eNext].GetVertex(1);
                        terminator.push_back(mPoints[GetVLabel(v)]);
                        if (v == vFirst)
                        {
                            return;
                        }
                    }
                    else
                    {
                        v = mEdges[eNext].GetVertex(0);
                        terminator.push_back(mPoints[GetVLabel(v)]);
                        if (v == vFirst)
                        {
                            return;
                        }
                    }

                    e = eNext;
                    break;
                }
            }
            LogAssert(j < jmax, "Unexpected condition.");
        }
        LogAssert(i < numEdges, "Unexpected condition.");
    }

    // If projection plane is Dot(N,X) = c where N is unit length, then the
    // application must ensure that Dot(N,eye) > c.  That is, the eye point is
    // on the side of the plane to which N points.  The application must also
    // specify two vectors U and V in the projection plane so that {U,V,N} is
    // a right-handed and orthonormal set (the matrix [U V N] is orthonormal
    // with determinant 1).  The origin of the plane is computed internally as
    // the closest point to the eye point (an orthogonal pyramid for the
    // perspective projection).  If all vertices P on the terminator satisfy
    // Dot(N,P) < Dot(N,eye), then the polyhedron is completely visible (in
    // the sense of perspective projection onto the viewing plane).  In this
    // case the silhouette is computed by projecting the terminator points
    // onto the viewing plane.  The return value of the function is 'true'
    // when this happens.  However, if at least one terminator point P
    // satisfies Dot(N,P) >= Dot(N,eye), then the silhouette is unbounded in
    // the view plane.  It is not computed and the function returns 'false'.
    // A silhouette point (x,y) is extracted from the point Q that is the
    // intersection of the ray whose origin is the eye point and that contains
    // a terminator point, Q = K+x*U+y*V+z*N where K is the origin of the
    // plane.
    bool ComputeSilhouette(gte::Vector3<Real> const& eye,
        gte::Plane3<Real> const& plane, gte::Vector3<Real> const& U,
        gte::Vector3<Real> const& V, V2Array& silhouette)
    {
        V3Array terminator;
        ComputeTerminator(eye, terminator);
        return ComputeSilhouette(terminator, eye, plane, U, V, silhouette);
    }

    bool ComputeSilhouette(V3Array& terminator, gte::Vector3<Real> const& eye,
        gte::Plane3<Real> const& plane, gte::Vector3<Real> const& U,
        gte::Vector3<Real> const& V, V2Array& silhouette)
    {
        Real eyeDistance = mQuery(eye, plane).signedDistance;
        LogAssert(eyeDistance > (Real)0, "The eye must be outside the polyhedron.");

        // The closest planar point to E is K = E - distance*N.
        gte::Vector3<Real> closest = eye - eyeDistance * plane.normal;

        // Project the polyhedron points onto the plane.
        for (auto const& point : terminator)
        {
            Real vertexDistance = mQuery(point, plane).signedDistance;
            if (vertexDistance >= eyeDistance)
            {
                // Cannot project the vertex onto the plane.
                return false;
            }

            // Compute projected point Q.
            Real ratio = eyeDistance / (eyeDistance - vertexDistance);
            gte::Vector3<Real> projected = eye + ratio * (point - eye);

            // Compute (x,y) so that Q = K + x*U + y*V + z*N.
            gte::Vector3<Real> diff = projected - closest;
            silhouette.push_back(gte::Vector2<Real>{ Dot(U, diff), Dot(V, diff) });
        }

        return true;
    }

    // Create an egg-shaped object that is axis-aligned and centered at
    // (xc,yc,zc).  The input bounds are all positive and represent the
    // distances from the center to the six extreme points on the egg.
    static void CreateEggShape(gte::Vector3<Real> const& center, Real x0,
        Real x1, Real y0, Real y1, Real z0, Real z1, int32_t maxSteps,
        ConvexPolyhedron& egg)
    {
        LogAssert(x0 > (Real)0 && x1 > (Real)0, "Invalid input.");
        LogAssert(y0 > (Real)0 && y1 > (Real)0, "Invalid input.");
        LogAssert(z0 > (Real)0 && z1 > (Real)0, "Invalid input.");
        LogAssert(maxSteps >= 0, "Invalid input.");

        // Start with an octahedron whose 6 vertices are (-x0,0,0), (x1,0,0),
        // (0,-y0,0), (0,y1,0), (0,0,-z0), (0,0,z1).  The center point will be
        // added later.
        V3Array points(6);
        points[0] = { -x0, (Real)0, (Real)0 };
        points[1] = { +x1, (Real)0, (Real)0 };
        points[2] = { (Real)0, -y0, (Real)0 };
        points[3] = { (Real)0, +y1, (Real)0 };
        points[4] = { (Real)0, (Real)0, -z0 };
        points[5] = { (Real)0, (Real)0, +z1 };

        IArray indices(24);
        indices[0] = 1;  indices[1] = 3;  indices[2] = 5;
        indices[3] = 3;  indices[4] = 0;  indices[5] = 5;
        indices[6] = 0;  indices[7] = 2;  indices[8] = 5;
        indices[9] = 2;  indices[10] = 1;  indices[11] = 5;
        indices[12] = 3;  indices[13] = 1;  indices[14] = 4;
        indices[15] = 0;  indices[16] = 3;  indices[17] = 4;
        indices[18] = 2;  indices[19] = 0;  indices[20] = 4;
        indices[21] = 1;  indices[22] = 2;  indices[23] = 4;

        egg.SetInitialELabel(0);
        egg.Create(points, indices);

        // Subdivide the triangles.  The midpoints of the edges are computed.
        // The triangle is replaced by four subtriangles using the original 3
        // vertices and the 3 new edge midpoints.
        for (int32_t step = 1; step <= maxSteps; ++step)
        {
            int32_t numVertices = egg.GetNumVertices();
            int32_t numEdges = egg.GetNumEdges();
            int32_t numTriangles = egg.GetNumTriangles();

            // Compute lifted edge midpoints.
            for (int32_t i = 0; i < numEdges; ++i)
            {
                // Get an edge.
                MTEdge const& edge = egg.GetEdge(i);
                int32_t v0 = egg.GetVLabel(edge.GetVertex(0));
                int32_t v1 = egg.GetVLabel(edge.GetVertex(1));

                // Compute lifted centroid to points.
                gte::Vector3<Real> lifted = egg.GetPoint(v0) + egg.GetPoint(v1);
                Real xr = (lifted[0] > (Real)0 ? lifted[0] / x1 : lifted[0] / x0);
                Real yr = (lifted[1] > (Real)0 ? lifted[1] / y1 : lifted[1] / y0);
                Real zr = (lifted[2] > (Real)0 ? lifted[2] / z1 : lifted[2] / z0);
                lifted *= (Real)1 / std::sqrt(xr * xr + yr * yr + zr * zr);

                // Add the point to the array.  Store the point index in the
                // edge label for support in adding new triangles.
                egg.SetELabel(i, numVertices);
                ++numVertices;
                egg.AddPoint(lifted);
            }

            // Add the new triangles and remove the old triangle.  The removal
            // in slot i will cause the last added triangle to be moved to
            // that slot.  This side effect will not interfere with the
            // iteration and removal of the triangles.
            for (int32_t i = 0; i < numTriangles; ++i)
            {
                MTTriangle const& triangle = egg.GetTriangle(i);
                int32_t v0 = egg.GetVLabel(triangle.GetVertex(0));
                int32_t v1 = egg.GetVLabel(triangle.GetVertex(1));
                int32_t v2 = egg.GetVLabel(triangle.GetVertex(2));
                int32_t v01 = egg.GetELabel(triangle.GetEdge(0));
                int32_t v12 = egg.GetELabel(triangle.GetEdge(1));
                int32_t v20 = egg.GetELabel(triangle.GetEdge(2));
                egg.Insert(v0, v01, v20);
                egg.Insert(v01, v1, v12);
                egg.Insert(v20, v12, v2);
                egg.Insert(v01, v12, v20);
                egg.Remove(v0, v1, v2);
            }
        }

        // Add the center.
        for (auto& point : egg.mPoints)
        {
            point += center;
        }

        egg.UpdatePlanes();
    }

    // Debugging support.
    virtual void Print(std::ofstream& output) const override
    {
        MTMesh::Print(output);

        output << "points:" << std::endl;
        int32_t const numPoints = static_cast<int32_t>(mPoints.size());
        for (int32_t i = 0; i < numPoints; ++i)
        {
            gte::Vector3<Real> const& point = mPoints[i];
            output << "point<" << i << "> = (";
            output << point[0] << ", ";
            output << point[1] << ", ";
            output << point[2] << ") ";
            output << std::endl;
        }
        output << std::endl;

        output << "planes:" << std::endl;
        int32_t const numPlanes = static_cast<int32_t>(mPlanes.size());
        for (int32_t i = 0; i < numPlanes; ++i)
        {
            gte::Plane3<Real> const& plane = mPlanes[i];
            output << "plane<" << i << "> = (";
            output << plane.normal[0] << ", ";
            output << plane.normal[1] << ", ";
            output << plane.normal[2] << ", ";
            output << plane.constant << ")";
            output << std::endl;
        }
        output << std::endl;
    }

    virtual bool Print(std::string const& filename) const override
    {
        std::ofstream output(filename);
        if (!output)
        {
            return false;
        }

        Print(output);
        return true;
    }

private:
    // A clipper for convex polyhedra against planes.
    class Clipper
    {
    public:
        class Vertex
        {
        public:
            Vertex()
                :
                point(gte::Vector3<Real>::Zero()),
                distance((Real)0),
                occurs(0),
                visible(true)
            {
            }

            gte::Vector3<Real> point;
            Real distance;
            int32_t occurs;
            bool visible;
        };

        class Edge
        {
        public:
            Edge()
                :
                visible(true)
            {
                vertex.fill(0);
                face.fill(0);
            }

            std::array<int32_t, 2> vertex;
            std::array<int32_t, 2> face;
            bool visible;
        };

        class Face
        {
        public:
            Face()
                :
                visible(true)
            {
            }

            gte::Plane3<Real> plane;
            std::set<int32_t> edges;
            bool visible;
        };

        // Construction.
        Clipper(ConvexPolyhedron<Real> const& polyhedron, Real epsilon = (Real)0)
            :
            mEpsilon(epsilon)
        {
            auto const& points = polyhedron.GetPoints();
            int32_t numVertices = polyhedron.GetNumVertices();
            mVertices.resize(numVertices);
            for (int32_t v = 0; v < numVertices; ++v)
            {
                mVertices[v].point = points[v];
            }

            int32_t numEdges = polyhedron.GetNumEdges();
            mEdges.resize(numEdges);
            for (int32_t e = 0; e < numEdges; ++e)
            {
                auto const& edge = polyhedron.GetEdge(e);
                for (int32_t i = 0; i < 2; ++i)
                {
                    mEdges[e].vertex[i] = polyhedron.GetVLabel(edge.GetVertex(i));
                    mEdges[e].face[i] = edge.GetTriangle(i);
                }
            }

            int32_t numTriangles = polyhedron.GetNumTriangles();
            mFaces.resize(numTriangles);
            for (int32_t t = 0; t < numTriangles; ++t)
            {
                mFaces[t].plane = polyhedron.GetPlane(t);
                auto const& triangle = polyhedron.GetTriangle(t);
                for (int32_t i = 0; i < 3; ++i)
                {
                    mFaces[t].edges.insert(triangle.GetEdge(i));
                }
            }
        }

        // Discard the portion of the mesh on the negative side of the plane.
        // This function is valid for any manifold triangle mesh (at most two
        // triangles shared per edge).
        int32_t Clip(gte::Plane3<Real> const& plane)
        {
            // Compute signed distances from vertices to plane.
            gte::DCPQuery<Real, gte::Vector3<Real>, gte::Plane3<Real>> query;
            int32_t numPositive = 0, numNegative = 0;
            for (auto& vertex : mVertices)
            {
                if (vertex.visible)
                {
                    vertex.distance = query(vertex.point, plane).signedDistance;
                    if (vertex.distance > mEpsilon)
                    {
                        ++numPositive;
                    }
                    else if (vertex.distance < -mEpsilon)
                    {
                        ++numNegative;
                        vertex.visible = false;
                    }
                    else
                    {
                        // The point is on the plane (within floating point
                        // tolerance).
                        vertex.distance = (Real)0;
                    }
                }
            }

            if (numPositive == 0)
            {
                // Mesh is in negative half-space, fully clipped.
                return -1;
            }

            if (numNegative == 0)
            {
                // Mesh is in positive half-space, fully visible.
                return +1;
            }

            // Clip the visible edges.
            int32_t const numEdges = static_cast<int32_t>(mEdges.size());
            for (int32_t e = 0; e < numEdges; ++e)
            {
                Edge& edge = mEdges[e];
                if (edge.visible)
                {
                    int32_t v0 = edge.vertex[0];
                    int32_t v1 = edge.vertex[1];
                    int32_t f0 = edge.face[0];
                    int32_t f1 = edge.face[1];
                    Face& face0 = mFaces[f0];
                    Face& face1 = mFaces[f1];
                    Real d0 = mVertices[v0].distance;
                    Real d1 = mVertices[v1].distance;

                    if (d0 <= (Real)0 && d1 <= (Real)0)
                    {
                        // The edge is culled.  If the edge is exactly on the
                        // clip plane, it is possible that a visible triangle
                        // shares it.  The edge will be re-added during the
                        // face loop.
                        face0.edges.erase(e);
                        if (face0.edges.empty())
                        {
                            face0.visible = false;
                        }

                        face1.edges.erase(e);
                        if (face1.edges.empty())
                        {
                            face1.visible = false;
                        }

                        edge.visible = false;
                        continue;
                    }

                    if (d0 >= (Real)0 && d1 >= (Real)0)
                    {
                        // Face retains the edge.
                        continue;
                    }

                    // The edge is split by the plane.  Compute the point of
                    // intersection.  If the old edge is <V0,V1> and I is the
                    // intersection point, the new edge is <V0,I> when d0 > 0
                    // or <I,V1> when d1 > 0.
                    int32_t vNew = static_cast<int32_t>(mVertices.size());
                    mVertices.push_back(Vertex());
                    Vertex& vertexNew = mVertices[vNew];

                    gte::Vector3<Real>& point0 = mVertices[v0].point;
                    gte::Vector3<Real>& point1 = mVertices[v1].point;
                    vertexNew.point = point0 + (d0 / (d0 - d1)) * (point1 - point0);

                    if (d0 > (Real)0)
                    {
                        edge.vertex[1] = vNew;
                    }
                    else
                    {
                        edge.vertex[0] = vNew;
                    }
                }
            }

            // The mesh straddles the plane.  A new convex polygonal face will
            // be generated.  Add it now and insert edges when they are visited.
            int32_t fNew = static_cast<int32_t>(mFaces.size());
            mFaces.push_back(Face());
            Face& faceNew = mFaces[fNew];
            faceNew.plane = plane;

            // Process the faces.
            for (int32_t f = 0; f < fNew; ++f)
            {
                Face& face = mFaces[f];
                if (face.visible)
                {
                    // Determine if the face is on the negative side, the
                    // positive side, or split by the clipping plane.  The
                    // occurs members are set to zero to help find the
                    // endpoints of the polyline that results from clipping
                    // a face.
                    LogAssert(face.edges.size() >= 2, "Unexpected condition.");
                    for (auto e : face.edges)
                    {
                        Edge& edge = mEdges[e];
                        LogAssert(edge.visible, "Unexpected condition.");
                        mVertices[edge.vertex[0]].occurs = 0;
                        mVertices[edge.vertex[1]].occurs = 0;
                    }

                    int32_t vStart, vFinal;
                    if (GetOpenPolyline(face, vStart, vFinal))
                    {
                        // Polyline is open, close it up.
                        int32_t eNew = static_cast<int32_t>(mEdges.size());
                        mEdges.push_back(Edge());
                        Edge& edgeNew = mEdges[eNew];

                        edgeNew.vertex[0] = vStart;
                        edgeNew.vertex[1] = vFinal;
                        edgeNew.face[0] = f;
                        edgeNew.face[1] = fNew;

                        // Add new edge to polygons.
                        face.edges.insert(eNew);
                        faceNew.edges.insert(eNew);
                    }
                }
            }

            // Process 'faceNew' to make sure it is a simple polygon
            // (theoretically convex, but numerically may be slightly not
            // convex).  Floating-point round-off errors can cause the new
            // face from the last loop to be needle-like with a collapse of
            // two edges into a single edge.  This block guarantees the
            // invariant "face always a simple polygon".
            Postprocess(fNew, faceNew);
            if (faceNew.edges.size() < 3)
            {
                // Face is completely degenerate, remove it from mesh.
                mFaces.pop_back();
            }

            return 0;
        }

        // Convert back to a convex polyhedron.
        void Convert(ConvexPolyhedron<Real>& polyhedron)
        {
            // Get visible vertices.
            int32_t numVertices = static_cast<int32_t>(mVertices.size());
            std::vector<gte::Vector3<Real>> points;
            std::vector<int32_t> vMap(numVertices);
            std::fill(vMap.begin(), vMap.end(), -1);
            for (int32_t v = 0; v < numVertices; ++v)
            {
                Vertex const& vertex = mVertices[v];
                if (vertex.visible)
                {
                    vMap[v] = static_cast<int32_t>(points.size());
                    points.push_back(vertex.point);
                }
            }

            std::vector<int32_t> indices;
            std::vector<gte::Plane3<Real>> planes;
            GetTriangles(indices, planes);

            // Reorder the indices.
            for (auto& index : indices)
            {
                int32_t oldC = index;
                LogAssert(0 <= oldC && oldC < numVertices, "Index out of range.");
                int32_t newC = vMap[oldC];
                LogAssert(0 <= newC && newC < static_cast<int32_t>(points.size()), "Index out of range.");
                index = newC;
            }

            polyhedron.Create(points, indices, planes);
        }

        // For debugging.
        bool Print(std::string const& filename) const
        {
            std::ofstream output(filename);
            if (!output)
            {
                return false;
            }

            int32_t const numVertices = static_cast<int32_t>(mVertices.size());
            output << numVertices << " vertices" << std::endl;
            for (int32_t v = 0; v < numVertices; ++v)
            {
                Vertex const& vertex = mVertices[v];
                output << "v<" << v << "> ";
                output << (vertex.visible ? 'T' : 'F') << " (";
                output << vertex.point[0] << ",";
                output << vertex.point[1] << ",";
                output << vertex.point[2] << ")";
                output << std::endl;
            }
            output << std::endl;

            int32_t const numEdges = static_cast<int32_t>(mEdges.size());
            output << numEdges << " edges" << std::endl;
            for (int32_t e = 0; e < numEdges; ++e)
            {
                Edge const& edge = mEdges[e];
                output << "e<" << e << "> ";
                output << (edge.visible ? 'T' : 'F') << " ";
                output << "v[" << edge.vertex[0] << "," << edge.vertex[1] << "], ";
                output << "t[" << edge.face[0] << "," << edge.face[1] << "]";
                output << std::endl;
            }
            output << std::endl;

            int32_t const numFaces = static_cast<int32_t>(mFaces.size());
            output << numFaces << " faces" << std::endl;
            for (int32_t f = 0; f < numFaces; ++f)
            {
                Face const& face = mFaces[f];
                output << "t<" << f << "> ";
                output << (face.visible ? 'T' : 'F') << " ";
                output << "e = ";
                for (auto const& edge : face.edges)
                {
                    output << edge << " ";
                }
                output << std::endl;
            }

            return true;
        }

    private:
        // Support for postprocessing faces.
        class EdgePlus
        {
        public:
            EdgePlus()
                :
                e(0), v0(0), v1(0), f0(0), f1(0)
            {
            }

            EdgePlus(int32_t inE, Edge const& edge)
                :
                e(inE)
            {
                f0 = edge.face[0];
                f1 = edge.face[1];

                if (edge.vertex[0] < edge.vertex[1])
                {
                    v0 = edge.vertex[0];
                    v1 = edge.vertex[1];
                }
                else
                {
                    v0 = edge.vertex[1];
                    v1 = edge.vertex[0];
                }
            }

            bool operator<(EdgePlus const& other) const
            {
                if (v1 < other.v1)
                {
                    return true;
                }

                if (v1 == other.v1)
                {
                    return v0 < other.v0;
                }

                return false;
            }

            bool operator==(EdgePlus const& other) const
            {
                return v0 == other.v0 && v1 == other.v1;
            }

            bool operator!=(EdgePlus const& other) const
            {
                return !operator==(other);
            }

            int32_t e, v0, v1, f0, f1;
        };

        void Postprocess(int32_t fNew, Face& faceNew)
        {
            int32_t const numEdges = static_cast<int32_t>(faceNew.edges.size());
            std::vector<EdgePlus> edges(numEdges);
            int32_t i = 0;
            for (auto e : faceNew.edges)
            {
                edges[i++] = EdgePlus(e, mEdges[e]);
            }
            std::sort(edges.begin(), edges.end());

            // Process duplicate edges.
            for (int32_t i0 = 0, i1 = 1; i1 < numEdges; i0 = i1++)
            {
                if (edges[i0] == edges[i1])
                {
                    // Found two equivalent edges (same vertex endpoints).
#ifdef _DEBUG
                    int32_t i2 = i1 + 1;
                    if (i2 < numEdges)
                    {
                        // Make sure an edge occurs at most twice.  If not,
                        // then algorithm needs to be modified to handle it.
                        LogAssert(edges[i1] != edges[i2], "Unexpected condition.");
                    }
#endif
                    // Edge E0 has vertices V0, V1 and faces F0, NF.  Edge E1
                    // has vertices V0, V1 and faces F1, NF.
                    int32_t e0 = edges[i0].e;
                    int32_t e1 = edges[i1].e;
                    Edge& edge0 = mEdges[e0];
                    Edge& edge1 = mEdges[e1];

                    // Remove E0 and E1 from faceNew.
                    faceNew.edges.erase(e0);
                    faceNew.edges.erase(e1);

                    // Remove faceNew from E0.
                    if (edge0.face[0] == fNew)
                    {
                        edge0.face[0] = edge0.face[1];
                    }
                    else
                    {
                        LogAssert(edge0.face[1] == fNew, "Unexpected condition.");
                    }
                    edge0.face[1] = -1;

                    // Remove faceNew from E1.
                    if (edge1.face[0] == fNew)
                    {
                        edge1.face[0] = edge1.face[1];
                    }
                    else
                    {
                        LogAssert(edge1.face[1] == fNew, "Unexpected condition.");
                    }
                    edge1.face[1] = -1;

                    // E2 is being booted from the system.  Update the face F1
                    // that shares it.  Update E1 to share F1.
                    int32_t f1 = edge1.face[0];
                    Face& face1 = mFaces[f1];
                    face1.edges.erase(e1);
                    face1.edges.insert(e0);
                    edge0.face[1] = f1;
                    edge1.visible = false;
                }
            }
        }

        bool GetOpenPolyline(Face& face, int32_t& vStart, int32_t& vFinal)
        {
            // Count the number of occurrences of each vertex in the polyline.
            bool okay = true;
            for (auto e : face.edges)
            {
                Edge& edge = mEdges[e];

                int32_t v0 = edge.vertex[0];
                ++mVertices[v0].occurs;
                if (mVertices[v0].occurs > 2)
                {
                    okay = false;
                }

                int32_t v1 = edge.vertex[1];
                ++mVertices[v1].occurs;
                if (mVertices[v1].occurs > 2)
                {
                    okay = false;
                }
            }

            if (!okay)
            {
#ifdef _DEBUG
                // If you reach this block, there is a good chance that
                // floating point round-off error had caused this face to be
                // needle-like and, what was theoretically a narrow V-shaped
                // portion (a vertex shared by two edges forming a small
                // angle), has collapsed into a single line segment.
                //
                // NOTE.  Once I added Postprocess, I have not gotten to this
                // block.
                std::ofstream output("error.txt");
                for (auto e : face.edges)
                {
                    Edge& edge = mEdges[e];
                    output << "e<" << e << "> = <" << edge.vertex[0] << ","
                        << edge.vertex[1] << "|" << edge.face[0] << ","
                        << edge.face[1] << "> ";
                    output << (edge.visible ? 'T' : 'F') << std::endl;
                }
                output.close();

                LogError("Probable numerical round-off errors caused this.");
#else
                return false;
#endif
            }

            // Determine whether the polyline is open.
            vStart = -1;
            vFinal = -1;
            for (auto e : face.edges)
            {
                Edge& edge = mEdges[e];

                int32_t v0 = edge.vertex[0];
                if (mVertices[v0].occurs == 1)
                {
                    if (vStart == -1)
                    {
                        vStart = v0;
                    }
                    else if (vFinal == -1)
                    {
                        vFinal = v0;
                    }
                    else
                    {
                        // If you reach this assert, there is a good chance
                        // that the polyhedron is not convex.  To check this,
                        // use the function ValidateHalfSpaceProperty() on
                        // your polyhedron right after you construct it.
                        LogError("Polyhedron might not be convex.");
                    }
                }

                int32_t v1 = edge.vertex[1];
                if (mVertices[v1].occurs == 1)
                {
                    if (vStart == -1)
                    {
                        vStart = v1;
                    }
                    else if (vFinal == -1)
                    {
                        vFinal = v1;
                    }
                    else
                    {
                        // If you reach this assert, there is a good chance
                        // that the polyhedron is not convex.  To check this,
                        // use the function ValidateHalfSpaceProperty() on
                        // your polyhedron right after you construct it.
                        LogError("Polyhedron might not be convex.");
                    }
                }
            }

            LogAssert((vStart == -1 && vFinal == -1)
                || (vStart != -1 && vFinal != -1), "Unexpected condition.");

            return vStart != -1;
        }

        void OrderVertices(Face& face, std::vector<int32_t>& vOrdered)
        {
            // Copy edge indices into contiguous memory.
            size_t const numEdges = face.edges.size();
            std::vector<int32_t> eOrdered(numEdges);
            size_t i = 0;
            for (auto const& e : face.edges)
            {
                eOrdered[i++] = e;
            }

            // Bubble sort (yes, it is...).
            for (size_t i0 = 0, i1 = 1, choice = 1; i1 < numEdges - 1; i0 = i1++)
            {
                Edge& edgeCurr = mEdges[eOrdered[i0]];
                int32_t curr = edgeCurr.vertex[choice];
                size_t j;
                for (j = i1; j < numEdges; ++j)
                {
                    Edge& edgeTemp = mEdges[eOrdered[j]];
                    if (edgeTemp.vertex[0] == curr)
                    {
                        std::swap(eOrdered[i1], eOrdered[j]);
                        choice = 1;
                        break;
                    }
                    if (edgeTemp.vertex[1] == curr)
                    {
                        std::swap(eOrdered[i1], eOrdered[j]);
                        choice = 0;
                        break;
                    }
                }
                LogAssert(j < numEdges, "Unexpected condition.");
            }

            vOrdered[0] = mEdges[eOrdered[0]].vertex[0];
            vOrdered[1] = mEdges[eOrdered[0]].vertex[1];
            for (i = 1; i < numEdges; ++i)
            {
                Edge& edge = mEdges[eOrdered[i]];
                if (edge.vertex[0] == vOrdered[i])
                {
                    vOrdered[i + 1] = edge.vertex[1];
                }
                else
                {
                    vOrdered[i + 1] = edge.vertex[0];
                }
            }
        }

        void GetTriangles(std::vector<int32_t>& indices, std::vector<gte::Plane3<Real>>& planes)
        {
            for (auto& face : mFaces)
            {
                if (face.visible)
                {
                    size_t const numEdges = face.edges.size();
                    LogAssert(numEdges >= 3, "Unexpected condition.");
                    std::vector<int32_t> vOrdered(numEdges + 1);
                    OrderVertices(face, vOrdered);

                    int32_t v0 = vOrdered[0];
                    int32_t v2 = vOrdered[numEdges - 1];
                    int32_t v1 = vOrdered[(numEdges - 1) >> 1];
                    gte::Vector3<Real> diff1 = mVertices[v1].point - mVertices[v0].point;
                    gte::Vector3<Real> diff2 = mVertices[v2].point - mVertices[v0].point;
                    Real sgnVolume = Dot(face.plane.normal, Cross(diff1, diff2));
                    if (sgnVolume > (Real)0)
                    {
                        // Clockwise, need to swap.
                        for (size_t i = 1; i + 1 < numEdges; ++i)
                        {
                            indices.push_back(v0);
                            indices.push_back(vOrdered[i + 1]);
                            indices.push_back(vOrdered[i]);
                            planes.push_back(face.plane);
                        }
                    }
                    else
                    {
                        // Counterclockwise.
                        for (size_t i = 1; i + 1 < numEdges; ++i)
                        {
                            indices.push_back(v0);
                            indices.push_back(vOrdered[i]);
                            indices.push_back(vOrdered[i + 1]);
                            planes.push_back(face.plane);
                        }
                    }
                }
            }
        }

        std::vector<Vertex> mVertices;
        std::vector<Edge> mEdges;
        std::vector<Face> mFaces;
        Real mEpsilon;
    };

private:
    // Support for intersection testing.
    static ConvexPolyhedron FindSolidIntersection(
        ConvexPolyhedron const& polyhedron0,
        ConvexPolyhedron const& polyhedron1)
    {
        ConvexPolyhedron intersection;
        if (polyhedron0.FindIntersection(polyhedron1, intersection))
        {
            return intersection;
        }

        // As surfaces, the polyhedra do not intersect.  However, as solids,
        // one polyhedron might be fully contained in the other.
        if (polyhedron0.ContainsPoint(polyhedron1.GetCentroid()))
        {
            intersection = polyhedron1;
            return intersection;
        }

        if (polyhedron1.ContainsPoint(polyhedron0.GetCentroid()))
        {
            intersection = polyhedron0;
            return intersection;
        }

        return intersection;
    }

    static int32_t GetHighBit(int32_t i)
    {
        // assert: i in [1,2^16].  That is, (i>0) && (0xFFFF0000&i)==0.
        // This is a binary search for the high-order bit of i.
        if ((i & 0xFF00) != 0)
        {
            if ((i & 0xF000) != 0)
            {
                if ((i & 0xC000) != 0)
                {
                    if ((i & 0x8000) != 0)
                    {
                        return 0x8000;
                    }
                    else // (i & 0x4000) != 0
                    {
                        return 0x4000;
                    }
                }
                else  // (i & 0x3000) != 0
                {
                    if ((i & 0x2000) != 0)
                    {
                        return 0x2000;
                    }
                    else  // (i & 0x1000) != 0
                    {
                        return 0x1000;
                    }
                }
            }
            else  // (i & 0x0F00) != 0
            {
                if ((i & 0x0C00) != 0)
                {
                    if ((i & 0x0800) != 0)
                    {
                        return 0x0800;
                    }
                    else  // (i & 0x0400) != 0
                    {
                        return 0x0400;
                    }
                }
                else  // (i & 0x0300) != 0
                {
                    if ((i & 0x0200) != 0)
                    {
                        return 0x0200;
                    }
                    else  // (i & 0x0100) != 0
                    {
                        return 0x0100;
                    }
                }
            }
        }
        else  // (i & 0x00FF)
        {
            if ((i & 0x00F0) != 0)
            {
                if ((i & 0x00C0) != 0)
                {
                    if ((i & 0x0080) != 0)
                    {
                        return 0x0080;
                    }
                    else  // (i & 0x0040) != 0
                    {
                        return 0x0040;
                    }
                }
                else  // (i & 0x0030) != 0
                {
                    if ((i & 0x0020) != 0)
                    {
                        return 0x0020;
                    }
                    else  // (i & 0x0010) != 0
                    {
                        return 0x0010;
                    }
                }
            }
            else  // (i & 0x000F) != 0
            {
                if ((i & 0x000C) != 0)
                {
                    if ((i & 0x0008) != 0)
                    {
                        return 0x0008;
                    }
                    else  // (i & 0x0004) != 0
                    {
                        return 0x0004;
                    }
                }
                else  // (i & 0x0003) != 0
                {
                    if ((i & 0x0002) != 0)
                    {
                        return 0x0002;
                    }
                    else  // (i & 0x0001) != 0
                    {
                        return 0x0001;
                    }
                }
            }
        }
    }

    // Support for computing surface area.
    Real GetTriangleArea(gte::Vector3<Real> const& normal,
        gte::Vector3<Real> const& vertex0, gte::Vector3<Real> const& vertex1,
        gte::Vector3<Real> const& vertex2) const
    {
        // Compute maximum absolute component of normal vector.
        int32_t maxIndex = 0;
        Real maxAbsValue = std::fabs(normal[0]);

        Real absValue = std::fabs(normal[1]);
        if (absValue > maxAbsValue)
        {
            maxIndex = 1;
            maxAbsValue = absValue;
        }

        absValue = std::fabs(normal[2]);
        if (absValue > maxAbsValue)
        {
            maxIndex = 2;
            maxAbsValue = absValue;
        }

        // Trap degenerate triangles.
        if (maxAbsValue == (Real)0)
        {
            return (Real)0;
        }

        // Compute area of projected triangle.
        Real d0, d1, d2, area;
        if (maxIndex == 0)
        {
            d0 = vertex1[2] - vertex2[2];
            d1 = vertex2[2] - vertex0[2];
            d2 = vertex0[2] - vertex1[2];
            area = std::fabs(vertex0[1] * d0 + vertex1[1] * d1 + vertex2[1] * d2);
        }
        else if (maxIndex == 1)
        {
            d0 = vertex1[0] - vertex2[0];
            d1 = vertex2[0] - vertex0[0];
            d2 = vertex0[0] - vertex1[0];
            area = std::fabs(vertex0[2] * d0 + vertex1[2] * d1 + vertex2[2] * d2);
        }
        else
        {
            d0 = vertex1[1] - vertex2[1];
            d1 = vertex2[1] - vertex0[1];
            d2 = vertex0[1] - vertex1[1];
            area = std::fabs(vertex0[0] * d0 + vertex1[0] * d1 + vertex2[0] * d2);
        }

        area *= (Real)0.5 / maxAbsValue;
        return area;
    }

    // Support for computing the terminator and silhouette.
    Real GetDistance(gte::Vector3<Real> const & eye, int32_t t, std::vector<Real>& distances) const
    {
        // Signed distance from eye to plane of triangle.  When distance is
        // positive, triangle is visible from eye (front-facing).  When
        // distance is negative, triangle is not visible from eye
        // (back-facing).  When distance is zero, triangle is visible
        // "on-edge" from eye.

        if (distances[t] == std::numeric_limits<Real>::max())
        {
            distances[t] = -mQuery(eye, mPlanes[t]).signedDistance;
        }

        return distances[t];
    }

    static bool IsNegativeProduct(Real distance0, Real distance1)
    {
        return (distance0 != (Real)0 ? (distance0 * distance1 <= (Real)0) :
            (distance1 != (Real)0));
    }

    V3Array mPoints;
    PArray mPlanes;
    gte::Vector3<Real> mCentroid;
    mutable gte::DCPQuery<Real, gte::Vector3<Real>, gte::Plane3<Real>> mQuery;
};
