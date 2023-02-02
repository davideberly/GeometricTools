// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Visual.h>
#include <Mathematics/Logger.h>
#include <cstdint>
using namespace gte;

Visual::Visual(
    std::shared_ptr<VertexBuffer> const& vbuffer,
    std::shared_ptr<IndexBuffer> const& ibuffer,
    std::shared_ptr<VisualEffect> const& effect)
    :
    mVBuffer(vbuffer),
    mIBuffer(ibuffer),
    mEffect(effect)
{
}

bool Visual::UpdateModelBound()
{
    LogAssert(mVBuffer != nullptr, "Buffer not attached.");

    std::set<uint32_t> required;
    required.insert(DF_R32G32B32_FLOAT);
    required.insert(DF_R32G32B32A32_FLOAT);
    char const* positions = mVBuffer->GetChannel(VASemantic::POSITION, 0, required);
    if (positions)
    {
        int32_t const numElements = mVBuffer->GetNumElements();
        int32_t const vertexSize = (int32_t)mVBuffer->GetElementSize();
        modelBound.ComputeFromData(numElements, vertexSize, positions);
        return true;
    }

    return false;
}

bool Visual::UpdateModelNormals()
{
    LogAssert(mVBuffer != nullptr && mIBuffer != nullptr, "Buffer not attached.");

    // Get vertex positions.
    std::set<uint32_t> required;
    required.insert(DF_R32G32B32_FLOAT);
    required.insert(DF_R32G32B32A32_FLOAT);
    char const* positions = mVBuffer->GetChannel(VASemantic::POSITION, 0, required);
    if (!positions)
    {
        return false;
    }

    // Get vertex normals.
    char* normals = mVBuffer->GetChannel(VASemantic::NORMAL, 0, required);
    if (!normals)
    {
        return false;
    }

    // Get triangle primitives.
    uint32_t primitiveType = mIBuffer->GetPrimitiveType();
    if ((primitiveType & IP_HAS_TRIANGLES) == 0)
    {
        // Normal vectors are not defined for point or segment primitives.
        return false;
    }

    uint32_t const numVertices = mVBuffer->GetNumElements();
    uint32_t const stride = (int32_t)mVBuffer->GetElementSize();
    uint32_t i;
    for (i = 0; i < numVertices; ++i)
    {
        Vector3<float>& normal = *(Vector3<float>*)(normals + static_cast<size_t>(i) * stride);
        normal = { 0.0f, 0.0f, 0.0f };
    }

    uint32_t const numTriangles = mIBuffer->GetNumPrimitives();
    bool isIndexed = mIBuffer->IsIndexed();
    for (i = 0; i < numTriangles; ++i)
    {
        // Get the vertex indices for the triangle.
        uint32_t v0, v1, v2;
        if (isIndexed)
        {
            mIBuffer->GetTriangle(i, v0, v1, v2);
        }
        else if (primitiveType == IP_TRIMESH)
        {
            v0 = 3 * i;
            v1 = v0 + 1;
            v2 = v0 + 2;
        }
        else  // primitiveType == IP_TRISTRIP
        {
            int32_t offset = (i & 1);
            v0 = i + offset;
            v1 = i + 1 + offset;
            v2 = i + 2 - offset;
        }

        // Get the vertex positions.
        Vector3<float> pos0 = *(Vector3<float>*)(positions + static_cast<size_t>(v0) * stride);
        Vector3<float> pos1 = *(Vector3<float>*)(positions + static_cast<size_t>(v1) * stride);
        Vector3<float> pos2 = *(Vector3<float>*)(positions + static_cast<size_t>(v2) * stride);

        // Compute the triangle normal.  The length of this normal is used
        // in the weighted sum of normals.
        Vector3<float> edge1 = pos1 - pos0;
        Vector3<float> edge2 = pos2 - pos0;
        Vector3<float> normal = Cross(edge1, edge2);

        // Add the triangle normal to the vertices' normal sums.
        Vector3<float>& nor0 = *(Vector3<float>*)(normals + static_cast<size_t>(v0) * stride);
        Vector3<float>& nor1 = *(Vector3<float>*)(normals + static_cast<size_t>(v1) * stride);
        Vector3<float>& nor2 = *(Vector3<float>*)(normals + static_cast<size_t>(v2) * stride);
        nor0 += normal;
        nor1 += normal;
        nor2 += normal;
    }

    // The vertex normals must be unit-length vectors.
    for (i = 0; i < numVertices; ++i)
    {
        Vector3<float>& normal = *(Vector3<float>*)(normals + static_cast<size_t>(i) * stride);
        if (normal != Vector3<float>::Zero())
        {
            Normalize(normal);
        }
    }

    return true;
}
