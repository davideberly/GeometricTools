// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.12

#include <Mathematics/Logger.h>
#include <Graphics/MeshFactory.h>
#include "RevolutionSurface.h"
using namespace gte;

RevolutionSurface::RevolutionSurface(
    std::shared_ptr<ParametricCurve<2, float>> const& curve,
    float xCenter, TopologyType topology, size_t numCurveSamples,
    size_t numRadialSamples, VertexFormat const& vformat,
    bool sampleByArcLength, bool outsideView, bool dynamicUpdate)
    :
    mCurve(curve),
    mXCenter(xCenter),
    mTopology(topology),
    mNumCurveSamples(numCurveSamples),
    mNumRadialSamples(numRadialSamples),
    mVFormat(vformat),
    mSampleByArcLength(sampleByArcLength),
    mOutsideView(outsideView),
    mDynamicUpdate(dynamicUpdate),
    mSamples(numCurveSamples),
    mSin(numRadialSamples + 1),
    mCos(numRadialSamples + 1),
    mNumVertices(0),
    mVertexSize(0),
    mPosData(nullptr),
    mSurface{}
{
    // Verify the preconditions for the vertex position.
    int32_t posIndex = mVFormat.GetIndex(VASemantic::POSITION, 0);
    LogAssert(
        posIndex >= 0,
        "The vertex format does not have POSITION in unit 0."
    );

    DFType type = mVFormat.GetType(posIndex);
    LogAssert(
        type == DF_R32G32B32_FLOAT,
        "The vertex format POSITION is not DF_R32G32B32_FLOAT."
    );

    ComputeSinCos();

    MeshFactory mf{};
    mf.SetVertexFormat(mVFormat);
    if (mDynamicUpdate)
    {
        mf.SetVertexBufferUsage(Resource::Usage::DYNAMIC_UPDATE);
    }
    mf.SetOutside(mOutsideView);

    // The topology of the meshes is all that matters. The vertices will be
    // modified later based on the curve of revolution.
    uint32_t uiNumCurveSamples = static_cast<uint32_t>(mNumCurveSamples);
    uint32_t uiNumRadialSamples = static_cast<uint32_t>(mNumRadialSamples);
    if (mTopology == REV_DISK_TOPOLOGY)
    {
        mSurface = mf.CreateDisk(uiNumCurveSamples, uiNumRadialSamples, 1.0f);
    }
    else if (mTopology == REV_CYLINDER_TOPOLOGY)
    {
        mSurface = mf.CreateCylinderOpen(uiNumCurveSamples, uiNumRadialSamples, 1.0f, 1.0f);
    }
    else if (mTopology == REV_SPHERE_TOPOLOGY)
    {
        mSurface = mf.CreateSphere(uiNumCurveSamples, uiNumRadialSamples, 1.0f);
    }
    else  // mTopology = REV_TORUS_TOPOLOGY
    {
        mSurface = mf.CreateTorus(uiNumCurveSamples, uiNumRadialSamples, 1.0f, 0.25f);
    }

    auto const& vbuffer = mSurface->GetVertexBuffer();
    char* vertices = vbuffer->GetData();
    mNumVertices = vbuffer->GetNumElements();
    mVertexSize = mVFormat.GetVertexSize();
    mPosData = vertices + mVFormat.GetOffset(posIndex);

    UpdateSurface();
}

void RevolutionSurface::UpdateSurface()
{
    float tMin = mCurve->GetTMin();
    float tRange = mCurve->GetTMax() - tMin;
    float denom = static_cast<float>(mNumCurveSamples) - 1.0f;
    float multiplier{};
    if (mSampleByArcLength)
    {
        multiplier = mCurve->GetTotalLength() / denom;
    }
    else
    {
        multiplier = tRange / denom;
    }

    // Sample the curve of revolution.
    for (size_t i = 0; i < mNumCurveSamples; ++i)
    {
        float delta = multiplier * static_cast<float>(i);
        float t{};
        if (mSampleByArcLength)
        {
            t = mCurve->GetTime(delta);
        }
        else
        {
            t = tMin + delta;
        }

        Vector2<float> position = mCurve->GetPosition(t);
        mSamples[i] = { position[0], 0.0f, position[1] };
    }

    // Store the samples and their rotated equivalents. The storage layout
    // is dependent on the topology of the mesh.
    if (mTopology == REV_DISK_TOPOLOGY)
    {
        UpdateDisk();
    }
    else if (mTopology == REV_CYLINDER_TOPOLOGY)
    {
        UpdateCylinder();
    }
    else if (mTopology == REV_SPHERE_TOPOLOGY)
    {
        UpdateSphere();
    }
    else  // mTopology = REV_TORUS_TOPOLOGY
    {
        UpdateTorus();
    }
}

void RevolutionSurface::UpdateDisk()
{
    // Get the initial ray.
    for (size_t c = 0; c < mNumCurveSamples; c++)
    {
        Position(c) = mSamples[c];
    }

    // The remaining rays are obtained by revolution.
    size_t numCurveSamplesM1 = mNumCurveSamples - 1;
    for (size_t r = 1; r < mNumRadialSamples; ++r)
    {
        for (size_t c = 1; c < mNumCurveSamples; ++c)
        {
            float radius = std::max(mSamples[c][0] - mXCenter, 0.0f);
            size_t i = c + numCurveSamplesM1 * r;
            Vector3<float> position
            {
                mXCenter + radius * mCos[r],
                radius * mSin[r],
                mSamples[c][2]
            };
            Position(i) = position;
        }
    }
}

void RevolutionSurface::UpdateSphere()
{
    // Set the South pole.
    Position(static_cast<size_t>(mNumVertices) - 2) = mSamples[0];

    // Set the north pole.
    Position(static_cast<size_t>(mNumVertices) - 1) = mSamples[mNumCurveSamples - 1];

    // Set the initial and final ray.
    for (size_t c = 1; c <= mNumCurveSamples - 2; ++c)
    {
        size_t i = (c - 1) * (mNumRadialSamples + 1);
        Position(i) = mSamples[c];
        i += mNumRadialSamples;
        Position(i) = mSamples[c];
    }

    // The remaining rays are obtained by revolution.
    for (size_t r = 1; r < mNumRadialSamples; ++r)
    {
        for (size_t c = 1; c <= mNumCurveSamples - 2; ++c)
        {
            float radius = std::max(mSamples[c][0] - mXCenter, 0.0f);
            size_t i = (c - 1) * (mNumRadialSamples + 1) + r;
            Vector3<float> position
            {
                mXCenter + radius * mCos[r],
                radius * mSin[r],
                mSamples[c][2]
            };
            Position(i) = position;
        }
    }
}

void RevolutionSurface::UpdateCylinder()
{
    // Set the initial and final ray.
    for (size_t c = 0; c < mNumCurveSamples; ++c)
    {
        size_t i = c * (mNumRadialSamples + 1);
        Position(i) = mSamples[c];
        i += mNumRadialSamples;
        Position(i) = mSamples[c];
    }

    // The remaining rays are obtained by revolution.
    for (size_t r = 1; r < mNumRadialSamples; ++r)
    {
        for (size_t c = 0; c < mNumCurveSamples; ++c)
        {
            float radius = std::max(mSamples[c][0] - mXCenter, 0.0f);
            size_t i = c * (mNumRadialSamples + 1) + r;
            Vector3<float> position
            {
                mXCenter + radius * mCos[r],
                radius * mSin[r],
                mSamples[c][2]
            };
            Position(i) = position;
        }
    }
}

void RevolutionSurface::UpdateTorus()
{
    // Set the initial and final ray.
    for (size_t c = 0; c < mNumCurveSamples; ++c)
    {
        size_t i = c * (mNumRadialSamples + 1);
        Position(i) = mSamples[c];
        i += mNumRadialSamples;
        Position(i) = mSamples[c];
    }

    // The remaining rays are obtained by revolution.
    for (size_t r = 1; r < mNumRadialSamples; ++r)
    {
        for (size_t c = 0; c < mNumCurveSamples; ++c)
        {
            float radius = std::max(mSamples[c][0] - mXCenter, 0.0f);
            size_t i = c * (mNumRadialSamples + 1) + r;
            Vector3<float> position
            {
                mXCenter + radius * mCos[r],
                radius * mSin[r],
                mSamples[c][2]
            };
            Position(i) = position;
        }
    }

    size_t i = static_cast<size_t>(mNumVertices) - (mNumRadialSamples + 1);
    for (size_t r = 0; r <= mNumRadialSamples; ++r, ++i)
    {
        Position(i) = Position(r);
    }
}

void RevolutionSurface::ComputeSinCos()
{
    // Compute slice vertex coefficients. The first and last coefficients
    // are duplicated to allow a closed cross section that has two different
    // pairs of texture coordinates at the shared vertex.
    float multiplier = static_cast<float>(GTE_C_TWO_PI) / static_cast<float>(mNumRadialSamples);
    for (size_t i = 0; i < mNumRadialSamples; ++i)
    {
        float angle = multiplier * static_cast<float>(i);
        mCos[i] = std::cos(angle);
        mSin[i] = std::sin(angle);
    }
    mSin[mNumRadialSamples] = mSin[0];
    mCos[mNumRadialSamples] = mCos[0];
}
