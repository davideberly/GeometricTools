// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.03.28

#pragma once

#include <Graphics/Visual.h>
#include <Mathematics/Mesh.h>
#include <cstdint>

// This class is a factory for Visual objects corresponding to common
// geometric primitives.  Triangle mesh primitives are generated.  Each mesh
// is centered at (0,0,0) and has an up-axis of (0,0,1).  The other axes
// forming the coordinate system are (1,0,0) and (0,1,0).
//
// The factory always generates 3-tuple positions.  If normals, tangents, or
// binormals are requested, they are also generated as 3-tuples.  They are
// stored in the vertex buffer as 3-tuples or 4-tuples as requested (w = 1 for
// positions, w = 0 for the others).  The factory also generates 2-tuple
// texture coordinates.  These are stored in the vertex buffer for 2-tuple
// units.  All other attribute types are unassigned by the factory.

namespace gte
{
    class MeshFactory
    {
    public:
        // Construction and destruction. The default vertex format uses
        // Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0) and immutable
        // vertices. The default index format uses 32-bit indices, and they
        // are immutable.
        MeshFactory();
        MeshFactory(VertexFormat const& vbFormat);
        MeshFactory(VertexFormat const& vbFormat, Resource::Usage vbUsage);
        ~MeshFactory() = default;

        // Specify the vertex format.
        inline void SetVertexFormat(VertexFormat const& format)
        {
            mVFormat = format;
        }

        // Specify the usage for the vertex buffer data.  The default is
        // Resource::Usage::IMMUTABLE.
        inline void SetVertexBufferUsage(Resource::Usage usage)
        {
            mVBUsage = usage;
        }

        // Specify the type of indices and where the index buffer data should
        // be stored.  For 'uint32_t' indices, set 'use32Bit' to 'true';
        // for 'uint16_t' indices, set 'use32Bit' to false.  The default
        // is 'uint32_t'.
        inline void SetIndexFormat(bool use32Bit)
        {
            mIndexSize = (use32Bit ? sizeof(uint32_t) : sizeof(uint16_t));
        }

        // Specify the usage for the index buffer data.  The default is
        // Resource::Usage::IMMUTABLE.
        inline void SetIndexBufferUsage(Resource::Usage usage)
        {
            mIBUsage = usage;
        }

        // For the geometric primitives that have an inside and an outside,
        // you may specify where the observer is expected to see the object.
        // If the observer must see the primitive from the outside, pass
        // 'true' to this function.  If the observer must see the primitive
        // from the inside, pass 'false'.  This Boolean flag simply controls
        // the triangle face order for face culling.  The default is 'true'
        // (observer view object from the outside).
        inline void SetOutside(bool outside)
        {
            mOutside = outside;
        }

        // The rectangle is in the plane z = 0 and is visible to an observer
        // who is on the side of the plane to which the normal (0,0,1) points.
        // It has corners (-xExtent, -yExtent, 0), (+xExtent, -yExtent, 0),
        // (-xExtent, +yExtent, 0), and (+xExtent, +yExtent, 0).  The mesh has
        // numXSamples vertices in the x-direction and numYSamples vertices in
        // the y-direction for a total of numXSamples*numYSamples vertices.
        std::shared_ptr<Visual> CreateRectangle(uint32_t numXSamples,
            uint32_t numYSamples, float xExtent, float yExtent);

        // The triangle is in the plane z = 0 and is visible to an observer
        // who is on the side of the plane to which the normal (0,0,1) points.
        // It has vertices (0, 0, 0), (xExtent, 0, 0), and (0, yExtent, 0).
        // The mesh has numSamples vertices along each of the x- and y-axes
        // for a total of numSamples*(numSamples+1)/2 vertices.
        std::shared_ptr<Visual> CreateTriangle(uint32_t numSamples,
            float xExtent, float yExtent);

        // The circular disk is in the plane z = 0 and is visible to an
        // observer who is on the side of the plane to which the normal
        // (0,0,1) points.  It has center (0,0,0) and the specified radius.
        // The mesh has its first vertex at the center.  Samples are placed
        // along rays whose common origin is the center.  There are
        // numRadialSamples rays.  Along each ray the mesh has
        // numShellSamples vertices.
        std::shared_ptr<Visual> CreateDisk(uint32_t numShellSamples,
            uint32_t numRadialSamples, float radius);

        // The box has center (0,0,0); unit-length axes (1,0,0), (0,1,0), and
        // (0,0,1); and extents (half-lengths) xExtent, yExtent, and zExtent.
        // The mesh has 8 vertices and 12 triangles.  For example, the box
        // corner in the first octant is (xExtent, yExtent, zExtent).
        std::shared_ptr<Visual> CreateBox(float xExtent, float yExtent,
            float zExtent);

        // The cylinder has center (0,0,0), the specified radius, and the
        // specified height.  The cylinder axis is a line segment of the form
        // (0,0,0) + t*(0,0,1) for |t| <= height/2.  The cylinder wall is
        // implicitly defined by x^2+y^2 = radius^2.  CreateCylinderOpen leads
        // to a cylinder whose end-disks are omitted; you have an open tube.
        // CreateCylinderClosed leads to a cylinder with end-disks.  Each
        // end-disk is a regular polygon that is tessellated by including a
        // vertex at the center of the polygon and decomposing the polygon
        // into triangles that all share the center vertex and each triangle
        // containing an edge of the polygon.
        std::shared_ptr<Visual> CreateCylinderOpen(uint32_t numAxisSamples,
            uint32_t numRadialSamples, float radius, float height);

        std::shared_ptr<Visual> CreateCylinderClosed(uint32_t numAxisSamples,
            uint32_t numRadialSamples, float radius, float height);

        // The sphere has center (0,0,0) and the specified radius.  The north
        // pole is at (0,0,radius) and the south pole is at (0,0,-radius).
        // The mesh has the topology of an open cylinder (which is also the
        // topology of a rectangle with wrap-around for one pair of parallel
        // edges) and is then stitched to the north and south poles.  The
        // triangles are unevenly distributed.  If you want a more even
        // distribution, create an icosahedron and subdivide it.
        std::shared_ptr<Visual> CreateSphere(uint32_t numZSamples,
            uint32_t numRadialSamples, float radius);

        // The torus has center (0,0,0).  If you observe the torus along the
        // line with direction (0,0,1), you will see an annulus.  The circle
        // that is the center of the annulus has radius 'outerRadius'.  The
        // distance from this circle to the boundaries of the annulus is the
        // 'inner radius'.
        std::shared_ptr<Visual> CreateTorus(uint32_t numCircleSamples,
            uint32_t numRadialSamples, float outerRadius, float innerRadius);

        // Platonic solids, all inscribed in a unit sphere centered at
        // (0,0,0).
        std::shared_ptr<Visual> CreateTetrahedron();
        std::shared_ptr<Visual> CreateHexahedron();
        std::shared_ptr<Visual> CreateOctahedron();
        std::shared_ptr<Visual> CreateDodecahedron();
        std::shared_ptr<Visual> CreateIcosahedron();

    private:
        // Support for creating vertex and index buffers.
        std::shared_ptr<VertexBuffer> CreateVBuffer(uint32_t numVertices);
        std::shared_ptr<IndexBuffer> CreateIBuffer(uint32_t numTriangles);

        // Support for vertex buffers.
        char* GetGeometricChannel(std::shared_ptr<VertexBuffer> const& vbuffer,
            VASemantic semantic, float w);

        inline Vector3<float>& Position(uint32_t i)
        {
            return *reinterpret_cast<Vector3<float>*>(mPositions + static_cast<size_t>(i) * mVFormat.GetVertexSize());
        }

        inline Vector3<float>& Normal(uint32_t i)
        {
            return *reinterpret_cast<Vector3<float>*>(mNormals + static_cast<size_t>(i) * mVFormat.GetVertexSize());
        }

        inline Vector3<float>& Tangent(uint32_t i)
        {
            return *reinterpret_cast<Vector3<float>*>(mTangents + static_cast<size_t>(i) * mVFormat.GetVertexSize());
        }

        inline Vector3<float>& Bitangent(uint32_t i)
        {
            return *reinterpret_cast<Vector3<float>*>(mBitangents + static_cast<size_t>(i) * mVFormat.GetVertexSize());
        }

        inline Vector2<float>& TCoord(uint32_t unit, uint32_t i)
        {
            return *reinterpret_cast<Vector2<float>*>(mTCoords[unit] + static_cast<size_t>(i) * mVFormat.GetVertexSize());
        }

        inline void SetPosition(uint32_t i, Vector3<float> const& pos)
        {
            Position(i) = pos;
        }

        void SetNormal(uint32_t i, Vector3<float> const& nor)
        {
            if (mNormals)
            {
                Normal(i) = nor;
            }
        }

        void SetTangent(uint32_t i, Vector3<float> const& tan)
        {
            if (mTangents)
            {
                Tangent(i) = tan;
            }
        }

        void SetBitangent(uint32_t i, Vector3<float> const& bin)
        {
            if (mBitangents)
            {
                Bitangent(i) = bin;
            }
        }

        void SetTCoord(uint32_t i, Vector2<float> const& tcd)
        {
            for (uint32_t unit = 0; unit < VAConstant::MAX_TCOORD_UNITS; ++unit)
            {
                if (mAssignTCoords[unit])
                {
                    TCoord(unit, i) = tcd;
                }
            }
        }

        void SetPlatonicTCoord(uint32_t i, Vector3<float> const& pos);

        // Support for index buffers.
        void ReverseTriangleOrder(IndexBuffer* ibuffer);

        VertexFormat mVFormat;
        size_t mIndexSize;
        Resource::Usage mVBUsage, mIBUsage;
        bool mOutside;
        std::array<bool, VAConstant::MAX_TCOORD_UNITS> mAssignTCoords;

        char* mPositions;
        char* mNormals;
        char* mTangents;
        char* mBitangents;
        std::array<char*, VAConstant::MAX_TCOORD_UNITS> mTCoords;
    };
}
