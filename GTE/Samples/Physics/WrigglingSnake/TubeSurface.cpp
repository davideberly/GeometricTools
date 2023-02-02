// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.11

#include <Mathematics/Logger.h>
#include <Mathematics/FrenetFrame.h>
#include "TubeSurface.h"
using namespace gte;

TubeSurface::TubeSurface(
    std::shared_ptr<ParametricCurve<3, float>> const& medial,
    std::shared_ptr<std::function<float(float)>> const& radial,
    Vector3<float> const& upVector, size_t numMedialSamples,
    size_t numSliceSamples, VertexFormat const& vformat,
    Vector2<float> const& tcoordMin, Vector2<float> const& tcoordMax,
    bool closed, bool sampleByArcLength, bool insideView, bool dynamicUpdate)
    :
    mMedial(medial),
    mRadial(radial),
    mUpVector(upVector),
    mNumMedialSamples(numMedialSamples),
    mNumSliceSamples(numSliceSamples),
    mVFormat(vformat),
    mTCoordMin(tcoordMin),
    mTCoordMax(tcoordMax),
    mClosed(closed),
    mSampleByArcLength(sampleByArcLength),
    mInsideView(insideView),
    mDynamicUpdate(dynamicUpdate),
    mSin(numSliceSamples + 1),
    mCos(numSliceSamples + 1),
    mNumVertices(0),
    mVertexSize(vformat.GetVertexSize()),
    mPosData(nullptr),
    mNorData(nullptr),
    mTcdData(nullptr),
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

    // Compute the surface vertices.
    uint32_t numTriangles{};
    if (mClosed)
    {
        mNumVertices =
            static_cast<uint32_t>((mNumSliceSamples + 1) * (mNumMedialSamples + 1));
        numTriangles =
            static_cast<uint32_t>(2 * mNumSliceSamples * mNumMedialSamples);
    }
    else
    {
        mNumVertices =
            static_cast<uint32_t>((mNumSliceSamples + 1) * mNumMedialSamples);
        numTriangles =
            static_cast<uint32_t>(2 * mNumSliceSamples * (mNumMedialSamples - 1));
    }

    // Create the surface vertices.
    auto vbuffer = std::make_shared<VertexBuffer>(mVFormat, mNumVertices);
    if (mDynamicUpdate)
    {
        vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    }

    // Compute the pointers to the vertex channels.
    char* vertices = vbuffer->GetData();
    mPosData = vertices + mVFormat.GetOffset(posIndex);
    int32_t norIndex = mVFormat.GetIndex(VASemantic::NORMAL, 0);
    if (norIndex >= 0)
    {
        type = mVFormat.GetType(norIndex);
        LogAssert(
            type == DF_R32G32B32_FLOAT,
            "The vertex format NORMAL is not DF_R32G32B32_FLOAT."
        );
        mNorData = vertices + mVFormat.GetOffset(norIndex);
    }
    int32_t tcdIndex = mVFormat.GetIndex(VASemantic::TEXCOORD, 0);
    if (tcdIndex >= 0)
    {
        type = mVFormat.GetType(tcdIndex);
        LogAssert(
            type == DF_R32G32_FLOAT,
            "The vertex format TEXCOORD is not DF_R32G32_FLOAT."
        );
        mTcdData = vertices + mVFormat.GetOffset(tcdIndex);
    }

    ComputeSinCos();
    ComputeVertices();

    if (mNorData)
    {
        ComputeNormals();
    }

    if (mTcdData)
    {
        ComputeUVs();
    }

    // Create the surface triangles.
    auto ibuffer = std::make_shared<IndexBuffer>(IPType::IP_TRIMESH, numTriangles, sizeof(uint32_t));
    ComputeIndices(ibuffer->Get<uint32_t>(), insideView);

    mSurface = std::make_shared<Visual>(vbuffer, ibuffer);
}

void TubeSurface::ComputeSinCos()
{
    // Compute slice vertex coefficients. The first and last coefficients
    // are duplicated to allow a closed cross section that has two different
    // pairs of texture coordinates at the shared vertex.
    float multiplier = static_cast<float>(GTE_C_TWO_PI) / static_cast<float>(mNumSliceSamples);
    for (size_t i = 0; i < mNumSliceSamples; ++i)
    {
        float angle = multiplier * static_cast<float>(i);
        mCos[i] = std::cos(angle);
        mSin[i] = std::sin(angle);
    }
    mSin[mNumSliceSamples] = mSin[0];
    mCos[mNumSliceSamples] = mCos[0];
}

void TubeSurface::ComputeVertices()
{
    float tMin = mMedial->GetTMin();
    float tRange = mMedial->GetTMax() - tMin;

    // Sampling by arc length requires the total length of the curve.
    float totalLength{};
    if (mSampleByArcLength)
    {
        totalLength = mMedial->GetTotalLength();
    }
    else
    {
        totalLength = 0.0f;
    }

    // Vertex construction requires a normalized time (uses a division).
    float denom{};
    if (mClosed)
    {
        denom = static_cast<float>(mNumMedialSamples);
    }
    else
    {
        denom = static_cast<float>(mNumMedialSamples) - 1.0f;
    }

    float multiplier{};
    if (mSampleByArcLength)
    {
        multiplier = totalLength / denom;
    }
    else
    {
        multiplier = tRange / denom;
    }

    for (size_t m = 0, v = 0; m < mNumMedialSamples; ++m, ++v)
    {
        float delta = multiplier * static_cast<float>(m);
        float t{};
        if (mSampleByArcLength)
        {
            t = mMedial->GetTime(delta);
        }
        else
        {
            t = tMin + delta;
        }

        float radius = (*mRadial)(t);

        // Compute frame.
        Vector3<float> position{}, tangent{}, normal{}, binormal{};
        if (mUpVector != Vector3<float>::Zero())
        {
            // Always use 'up' vector N rather than curve normal. You must
            // constrain the curve so that T and N are never parallel. To
            // build the frame from this, let
            //   B = Cross(T,N)/Length(Cross(T,N))
            // and replace
            //   N = Cross(B,T)/Length(Cross(B,T))
            position = mMedial->GetPosition(t);
            tangent = mMedial->GetTangent(t);
            binormal = UnitCross(tangent, mUpVector);
            normal = UnitCross(binormal, tangent);
        }
        else
        {
            // Use Frenet frame to create slices.
            FrenetFrame3<float> frenet(mMedial);
            frenet(t, position, tangent, normal, binormal);
        }

        // Compute slice vertices, duplication at end point as noted earlier.
        size_t vSave = v;
        for (size_t i = 0; i < mNumSliceSamples; ++i, ++v)
        {
            Position(v) = position + radius * (mCos[i] * normal + mSin[i] * binormal);
        }
        Position(v) = Position(vSave);
    }

    if (mClosed)
    {
        for (size_t i = 0; i <= mNumSliceSamples; ++i)
        {
            size_t i1 = Index(i, mNumMedialSamples);
            size_t i0 = Index(i, 0);
            Position(i1) = Position(i0);
        }
    }
}

void TubeSurface::ComputeNormals()
{
    // Compute the interior normals (central differences).
    for (size_t m = 1; m + 2 <= mNumMedialSamples; ++m)
    {
        for (size_t s = 0; s < mNumSliceSamples; ++s)
        {
            size_t sM1 = (s > 0 ? s - 1 : mNumSliceSamples - 1);
            size_t sP1 = s + 1;
            size_t mM1 = m - 1;
            size_t mP1 = m + 1;
            Vector3<float> pos00 = Position(Index(sM1, m));
            Vector3<float> pos01 = Position(Index(sP1, m));
            Vector3<float> pos10 = Position(Index(s, mM1));
            Vector3<float> pos11 = Position(Index(s, mP1));
            Vector3<float> dir0 = pos00 - pos01;
            Vector3<float> dir1 = pos10 - pos11;
            Normal(Index(s, m)) = UnitCross(dir0, dir1);
        }
        Normal(Index(mNumSliceSamples, m)) = Normal(Index(0, m));
    }

    // Compute the boundary normals.
    if (mClosed)
    {
        // Compute with central differences.
        for (size_t s = 0; s < mNumSliceSamples; ++s)
        {
            size_t sM1 = (s > 0 ? s - 1 : mNumSliceSamples - 1);
            size_t sP1 = s + 1;

            // m = 0
            Vector3<float> pos00 = Position(Index(sM1, 0));
            Vector3<float> pos01 = Position(Index(sP1, 0));
            Vector3<float> pos10 = Position(Index(s, mNumMedialSamples - 1));
            Vector3<float> pos11 = Position(Index(s, 1));
            Vector3<float> dir0 = pos00 - pos01;
            Vector3<float> dir1 = pos10 - pos11;
            Normal(Index(s, 0)) = UnitCross(dir0, dir1);

            // m = max
            Normal(Index(s, mNumMedialSamples)) = Normal(Index(s, 0));
        }
        Normal(Index(mNumSliceSamples, 0)) = Normal(Index(0, 0));
        Normal(Index(mNumSliceSamples, mNumMedialSamples)) =
            Normal(Index(0, mNumMedialSamples));
    }
    else
    {
        // Compute with one-sided differences.

        // m = 0
        for (size_t s = 0; s < mNumSliceSamples; ++s)
        {
            size_t sM1 = (s > 0 ? s - 1 : mNumSliceSamples - 1);
            size_t sP1 = s + 1;
            Vector3<float> pos00 = Position(Index(sM1, 0));
            Vector3<float> pos01 = Position(Index(sP1, 0));
            Vector3<float> pos10 = Position(Index(s, 0));
            Vector3<float> pos11 = Position(Index(s, 1));
            Vector3<float> dir0 = pos00 - pos01;
            Vector3<float> dir1 = pos10 - pos11;
            Normal(Index(s, 0)) = UnitCross(dir0, dir1);
        }
        Normal(Index(mNumSliceSamples, 0)) = Normal(Index(0, 0));

        // m = max-1
        for (size_t s = 0; s < mNumSliceSamples; ++s)
        {
            size_t sM1 = (s > 0 ? s - 1 : mNumSliceSamples - 1);
            size_t sP1 = s + 1;
            Vector3<float> pos00 = Position(Index(sM1, mNumMedialSamples - 1));
            Vector3<float> pos01 = Position(Index(sP1, mNumMedialSamples - 1));
            Vector3<float> pos10 = Position(Index(s, mNumMedialSamples - 2));
            Vector3<float> pos11 = Position(Index(s, mNumMedialSamples - 1));
            Vector3<float> dir0 = pos00 - pos01;
            Vector3<float> dir1 = pos10 - pos11;
            Normal(Index(s, 0)) = UnitCross(dir0, dir1);
        }
        Normal(Index(mNumSliceSamples, mNumMedialSamples - 1)) =
            Normal(Index(0, mNumMedialSamples - 1));
    }
}

void TubeSurface::ComputeUVs()
{
    Vector2<float> tcoordRange = mTCoordMax - mTCoordMin;
    size_t mmax = (mClosed ? mNumMedialSamples : mNumMedialSamples - 1);
    for (size_t m = 0, v = 0; m <= mmax; m++)
    {
        float mratio = static_cast<float>(m) / static_cast<float>(mmax);
        float mvalue = mTCoordMin[1] + mratio * tcoordRange[1];
        for (size_t s = 0; s <= mNumSliceSamples; ++s, ++v)
        {
            float sratio = static_cast<float>(s) / static_cast<float>(mNumSliceSamples);
            float svalue = mTCoordMin[0] + sratio * tcoordRange[0];
            TCoord(v) = { svalue, mvalue };
        }
    }
}

void TubeSurface::ComputeIndices(uint32_t* indices, bool insideView)
{
    uint32_t mstart = 0;
    for (size_t m = 0; m + 1 < mNumMedialSamples; ++m)
    {
        uint32_t i0 = mstart;
        uint32_t i1 = i0 + 1;
        mstart += static_cast<uint32_t>(mNumSliceSamples + 1);
        uint32_t i2 = mstart;
        uint32_t i3 = i2 + 1;
        for (uint32_t i = 0; i < mNumSliceSamples; ++i, indices += 6)
        {
            if (insideView)
            {
                indices[0] = i0++;
                indices[1] = i2;
                indices[2] = i1;
                indices[3] = i1++;
                indices[4] = i2++;
                indices[5] = i3++;
            }
            else  // outside view
            {
                indices[0] = i0++;
                indices[1] = i1;
                indices[2] = i2;
                indices[3] = i1++;
                indices[4] = i3++;
                indices[5] = i2++;
            }
        }
    }

    if (mClosed)
    {
        uint32_t i0 = mstart;
        uint32_t i1 = i0 + 1;
        uint32_t i2 = 0;
        uint32_t i3 = i2 + 1;
        for (uint32_t i = 0; i < mNumSliceSamples; ++i, indices += 6)
        {
            if (insideView)
            {
                indices[0] = i0++;
                indices[1] = i2;
                indices[2] = i1;
                indices[3] = i1++;
                indices[4] = i2++;
                indices[5] = i3++;
            }
            else  // outside view
            {
                indices[0] = i0++;
                indices[1] = i1;
                indices[2] = i2;
                indices[3] = i1++;
                indices[4] = i3++;
                indices[5] = i2++;
            }
        }
    }
}

void TubeSurface::GetTMinSlice(std::vector<Vector3<float>>& slice)
{
    slice.resize(mNumSliceSamples + 1);
    for (size_t i = 0; i <= mNumSliceSamples; ++i)
    {
        slice[i] = Position(i);
    }
}

void TubeSurface::GetTMaxSlice(std::vector<Vector3<float>>& slice)
{
    slice.resize(mNumSliceSamples + 1);
    size_t j = static_cast<size_t>(mNumVertices - mNumSliceSamples - 1);
    for (size_t i = 0; i <= mNumSliceSamples; ++i, ++j)
    {
        slice[i] = Position(j);
    }
}

void TubeSurface::UpdateSurface()
{
    ComputeVertices();

    if (mNorData)
    {
        ComputeNormals();
    }
}
