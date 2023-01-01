// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.20

#pragma once

#include <Mathematics/Triangle.h>
#include <Graphics/Visual.h>

namespace gte
{
    class CollisionMesh
    {
    public:
        // The input 'mesh' must have a vertex buffer whose vertex format
        // has first binding call
        //   vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
        // or
        //   vformat.Bind(VASemantic::POSITION, DF_R32G32B32A_FLOAT, 0);
        // The input 'mesh' must have an index buffer whose primitive type
        // is IPType::IP_TRIMESH.
        CollisionMesh(std::shared_ptr<Visual> const& mesh);

        size_t GetNumVertices() const;
        Vector3<float> GetPosition(size_t i) const;
        size_t GetNumTriangles() const;
        bool GetTriangle(size_t t, std::array<int32_t, 3>& indices) const;
        bool GetModelTriangle(size_t t, Triangle3<float>& modelTriangle) const;
        bool GetWorldTriangle(size_t t, Triangle3<float>& worldTriangle) const;
        Matrix4x4<float> const& GetWorldTransform() const;

    private:
        // The triangle mesh passed to the constructor.
        std::shared_ptr<Visual> mMesh;

        // This is commonly accessed data from mMesh.
        std::shared_ptr<VertexBuffer> mVBuffer;
        std::shared_ptr<IndexBuffer> mIBuffer;
        size_t mVertexSize;
    };
}
