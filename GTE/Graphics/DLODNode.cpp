// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.03.03

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/DLODNode.h>
#include <Mathematics/Logger.h>
using namespace gte;

DLODNode::DLODNode(int32_t numLevelsOfDetail)
    :
    mModelLODCenter(Vector4<float>::Zero()),
    mWorldLODCenter(Vector4<float>::Zero()),
    mNumLevelsOfDetail(numLevelsOfDetail),
    mModelMinDistance{},
    mModelMaxDistance{},
    mWorldMinDistance{},
    mWorldMaxDistance{}
{
    LogAssert(
        mNumLevelsOfDetail > 0,
        "Invalid number of levels of detail.");

    mModelMinDistance.resize(mNumLevelsOfDetail, 0.0f);
    mModelMaxDistance.resize(mNumLevelsOfDetail, 0.0f);
    mWorldMinDistance.resize(mNumLevelsOfDetail, 0.0f);
    mWorldMaxDistance.resize(mNumLevelsOfDetail, 0.0f);
}

float DLODNode::GetModelMinDistance(int32_t i) const
{
    LogAssert(
        0 <= i && i < mNumLevelsOfDetail,
        "Invalid index in GetModelMinDistance.");

    return mModelMinDistance[i];
}

float DLODNode::GetModelMaxDistance(int32_t i) const
{
    LogAssert(
        0 <= i && i < mNumLevelsOfDetail,
        "Invalid index in GetModelMaxDistance.");

    return mModelMaxDistance[i];
}

float DLODNode::GetWorldMinDistance(int32_t i) const
{
    LogAssert(
        0 <= i && i < mNumLevelsOfDetail,
        "Invalid index in GetWorldMinDistance.");

    return mWorldMinDistance[i];
}

float DLODNode::GetWorldMaxDistance(int32_t i) const
{
    LogAssert(
        0 <= i && i < mNumLevelsOfDetail,
        "Invalid index in GetWorldMaxDistance.");

    return mWorldMaxDistance[i];
}

void DLODNode::SetModelDistance(int32_t i, float minDistance, float maxDistance)
{
    LogAssert(
        0 <= i && i < mNumLevelsOfDetail,
        "Invalid index in SetModelDistance.");

    LogAssert(
        minDistance < maxDistance,
        "Invalid range of distances in SetModelDistance.");

    mModelMinDistance[i] = minDistance;
    mModelMaxDistance[i] = maxDistance;
    mWorldMinDistance[i] = minDistance;
    mWorldMaxDistance[i] = maxDistance;
}

void DLODNode::SelectLevelOfDetail(std::shared_ptr<Camera> const& camera)
{
    // The child array of a DLODNode is compact; that is, there are no empty
    // slots in the array and the number of children is mChild.size().
    // Moreover, it is assumed that all model distance values were set for
    // these children.
    LogAssert(
        mChild.size() == static_cast<size_t>(mNumLevelsOfDetail),
        "Invalid DLODNode detected by SelectLevelOfDetail.");

    for (auto const& child : mChild)
    {
        LogAssert(
            child != nullptr,
            "Invalid DLODNode child detected by SelectLevelOfDetail.");
    }

    // Compute the world LOD center.
    mWorldLODCenter = DoTransform(worldTransform.GetHMatrix(), mModelLODCenter);

    // Compute the world squared-distance intervals.
    for (int32_t i = 0; i < mNumLevelsOfDetail; ++i)
    {
        mWorldMinDistance[i] = worldTransform.GetUniformScale() * mModelMinDistance[i];
        mWorldMaxDistance[i] = worldTransform.GetUniformScale() * mModelMaxDistance[i];
    }

    // Select the LOD child.
    SetActiveChild(SwitchNode::invalidChild);
    Vector4<float> diff = mWorldLODCenter - camera->GetPosition();
    float distance = Length(diff);
    for (int32_t i = 0; i < mNumLevelsOfDetail; ++i)
    {
        if (mWorldMinDistance[i] <= distance && distance < mWorldMaxDistance[i])
        {
            SetActiveChild(i);
            break;
        }
    }
}

void DLODNode::GetVisibleSet(Culler& culler, std::shared_ptr<Camera> const& camera,
    bool noCull)
{
    SelectLevelOfDetail(camera);
    SwitchNode::GetVisibleSet(culler, camera, noCull);
}
