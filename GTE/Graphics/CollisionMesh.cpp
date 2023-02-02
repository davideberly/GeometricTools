// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.20

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/CollisionMesh.h>
using namespace gte;

CollisionMesh::CollisionMesh(std::shared_ptr<Visual> const& mesh)
    :
    mMesh(mesh),
    mVBuffer{},
    mIBuffer{},
    mVertexSize(0)
{
    LogAssert(
        mMesh,
        "The input mesh must exist.");

    mVBuffer = mMesh->GetVertexBuffer();
    LogAssert(
        mVBuffer->GetNumElements() >= 3,
        "The vertex buffer does not have enough elements.");

    auto const& vformat = mVBuffer->GetFormat();
    LogAssert(
        vformat.GetNumAttributes() > 0,
        "The vertex format must have attributes.");

    VASemantic semantic{};
    DFType type{};
    uint32_t unit{};
    uint32_t offset{};
    vformat.GetAttribute(0, semantic, type, unit, offset);
    LogAssert(
        semantic == VASemantic::POSITION &&
        (type == DF_R32G32B32_FLOAT || type == DF_R32G32B32A32_FLOAT) &&
        unit == 0 &&
        offset == 0,
        "The vertex format does not satisfy the requirements.");

    mIBuffer = mMesh->GetIndexBuffer();
    LogAssert(
        mIBuffer->GetNumElements() > 0,
        "The index buffer does not have enough elements.");

    LogAssert(
        mIBuffer->GetPrimitiveType() == IPType::IP_TRIMESH,
        "The index buffer must represent a triangle mesh.");

    mVertexSize = static_cast<size_t>(vformat.GetVertexSize());
}

size_t CollisionMesh::GetNumVertices() const
{
    return static_cast<size_t>(mVBuffer->GetNumElements());
}

Vector3<float> CollisionMesh::GetPosition(size_t i) const
{
    char const* vertex = mVBuffer->GetData() + i * mVertexSize;
    return *reinterpret_cast<Vector3<float> const*>(vertex);
}

size_t CollisionMesh::GetNumTriangles() const
{
    return static_cast<size_t>(mIBuffer->GetNumPrimitives());
}

bool CollisionMesh::GetTriangle(size_t t, std::array<int32_t, 3>& indices) const
{
    if (t < GetNumTriangles())
    {
        uint32_t v0{}, v1{}, v2{};
        mIBuffer->GetTriangle(static_cast<uint32_t>(t), v0, v1, v2);
        indices[0] = static_cast<int32_t>(v0);
        indices[1] = static_cast<int32_t>(v1);
        indices[2] = static_cast<int32_t>(v2);
        return true;
    }
    return false;
}

bool CollisionMesh::GetModelTriangle(size_t t, Triangle3<float>& modelTriangle) const
{
    std::array<int32_t, 3> indices{};
    if (GetTriangle(t, indices))
    {
        for (size_t j = 0; j < 3; ++j)
        {
            modelTriangle.v[j] = GetPosition(indices[j]);
        }
        return true;
    }
    return false;
}

bool CollisionMesh::GetWorldTriangle(size_t t, Triangle3<float>& worldTriangle) const
{
    std::array<int32_t, 3> indices{};
    if (GetTriangle(t, indices))
    {
        Matrix4x4<float> const& hmatrix = GetWorldTransform();
        Vector3<float> modelVertex{};
        for (size_t j = 0; j < 3; ++j)
        {
            modelVertex = GetPosition(indices[j]);
            worldTriangle.v[j] = HProject(hmatrix * HLift(modelVertex, 1.0f));
        }
        return true;
    }
    return false;
}

Matrix4x4<float> const& CollisionMesh::GetWorldTransform() const
{
    return mMesh->worldTransform.GetHMatrix();
}
