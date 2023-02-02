// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "CpuMassSpringVolume.h"

CpuMassSpringVolume::CpuMassSpringVolume(std::shared_ptr<ProgramFactory> const&,
    int32_t numColumns, int32_t numRows, int32_t numSlices, float step, float viscosity,
    Environment&, bool& created)
    :
    mNumColumns(numColumns),
    mNumRows(numRows),
    mNumSlices(numSlices),
    mNumSliceElements(numColumns * numRows),
    mNumVolumeElements(numColumns * numRows * numSlices),
    mStep(step),
    mHalfStep(step / 2.0f),
    mSixthStep(step / 6.0f),
    mViscosity(viscosity),
    mMass(mNumVolumeElements),
    mInvMass(mNumVolumeElements),
    mPosition(mNumVolumeElements),
    mVelocity(mNumVolumeElements),
    mConstantC(mNumVolumeElements),
    mLengthC(mNumVolumeElements),
    mConstantR(mNumVolumeElements),
    mLengthR(mNumVolumeElements),
    mConstantS(mNumVolumeElements),
    mLengthS(mNumVolumeElements),
    mPTmp(mNumVolumeElements),
    mVTmp(mNumVolumeElements),
    mPAllTmp(mNumVolumeElements),
    mVAllTmp(mNumVolumeElements)
{
    created = true;
}

void CpuMassSpringVolume::SetMass(int32_t c, int32_t r, int32_t s, float mass)
{
    int32_t i = GetIndex(c, r, s);
    if (0.0f < mass && mass < std::numeric_limits<float>::max())
    {
        mMass[i] = mass;
        mInvMass[i] = 1.0f / mass;
    }
    else
    {
        mMass[i] = std::numeric_limits<float>::max();
        mInvMass[i] = 0.0f;
    }
}

void CpuMassSpringVolume::SetPosition(int32_t c, int32_t r, int32_t s,
    Vector3<float> const& position)
{
    mPosition[GetIndex(c, r, s)] = position;
}

void CpuMassSpringVolume::SetVelocity(int32_t c, int32_t r, int32_t s,
    Vector3<float> const& velocity)
{
    mVelocity[GetIndex(c, r, s)] = velocity;
}

void CpuMassSpringVolume::SetConstantC(int32_t c, int32_t r, int32_t s, float v)
{
    mConstantC[GetIndex(c, r, s)] = v;
}

void CpuMassSpringVolume::SetLengthC(int32_t c, int32_t r, int32_t s, float v)
{
    mLengthC[GetIndex(c, r, s)] = v;
}

void CpuMassSpringVolume::SetConstantR(int32_t c, int32_t r, int32_t s, float v)
{
    mConstantR[GetIndex(c, r, s)] = v;
}

void CpuMassSpringVolume::SetLengthR(int32_t c, int32_t r, int32_t s, float v)
{
    mLengthR[GetIndex(c, r, s)] = v;
}

void CpuMassSpringVolume::SetConstantS(int32_t c, int32_t r, int32_t s, float v)
{
    mConstantS[GetIndex(c, r, s)] = v;
}

void CpuMassSpringVolume::SetLengthS(int32_t c, int32_t r, int32_t s, float v)
{
    mLengthS[GetIndex(c, r, s)] = v;
}

Vector3<float> CpuMassSpringVolume::GetPosition(int32_t c, int32_t r, int32_t s) const
{
    return mPosition[GetIndex(c, r, s)];
}

std::vector<Vector3<float>>& CpuMassSpringVolume::GetPosition()
{
    return mPosition;
}

void CpuMassSpringVolume::Update(float time)
{
    // Runge-Kutta fourth-order solver.
    float halfTime = time + mHalfStep;
    float fullTime = time + mStep;

    // Compute the first step.
    int32_t c, r, s, i;
    for (s = 0, i = 0; s < mNumSlices; ++s)
    {
        for (r = 0; r < mNumRows; ++r)
        {
            for (c = 0; c < mNumColumns; ++c, ++i)
            {
                if (mInvMass[i] > 0.0f)
                {
                    mPAllTmp[i].d1 = mVelocity[i];
                    mVAllTmp[i].d1 = Acceleration(i, c, r, s, time,
                        mPosition, mVelocity);
                }
            }
        }
    }
    for (s = 0, i = 0; s < mNumSlices; ++s)
    {
        for (r = 0; r < mNumRows; ++r)
        {
            for (c = 0; c < mNumColumns; ++c, ++i)
            {
                if (mInvMass[i] > 0.0f)
                {
                    mPTmp[i] = mPosition[i] + mHalfStep*mPAllTmp[i].d1;
                    mVTmp[i] = mVelocity[i] + mHalfStep*mVAllTmp[i].d1;
                }
                else
                {
                    mPTmp[i] = mPosition[i];
                    mVTmp[i] = Vector3<float>::Zero();
                }
            }
        }
    }

    // Compute the second step.
    for (s = 0, i = 0; s < mNumSlices; ++s)
    {
        for (r = 0; r < mNumRows; ++r)
        {
            for (c = 0; c < mNumColumns; ++c, ++i)
            {
                if (mInvMass[i] > 0.0f)
                {
                    mPAllTmp[i].d2 = mVelocity[i];
                    mVAllTmp[i].d2 = Acceleration(i, c, r, s, halfTime,
                        mPTmp, mVTmp);
                }
            }
        }
    }
    for (s = 0, i = 0; s < mNumSlices; ++s)
    {
        for (r = 0; r < mNumRows; ++r)
        {
            for (c = 0; c < mNumColumns; ++c, ++i)
            {
                if (mInvMass[i] > 0.0f)
                {
                    mPTmp[i] = mPosition[i] + mHalfStep*mPAllTmp[i].d2;
                    mVTmp[i] = mVelocity[i] + mHalfStep*mVAllTmp[i].d2;
                }
                else
                {
                    mPTmp[i] = mPosition[i];
                    mVTmp[i] = Vector3<float>::Zero();
                }
            }
        }
    }

    // Compute the third step.
    for (s = 0, i = 0; s < mNumSlices; ++s)
    {
        for (r = 0; r < mNumRows; ++r)
        {
            for (c = 0; c < mNumColumns; ++c, ++i)
            {
                if (mInvMass[i] > 0.0f)
                {
                    mPAllTmp[i].d3 = mVelocity[i];
                    mVAllTmp[i].d3 = Acceleration(i, c, r, s, halfTime,
                        mPTmp, mVTmp);
                }
            }
        }
    }
    for (s = 0, i = 0; s < mNumSlices; ++s)
    {
        for (r = 0; r < mNumRows; ++r)
        {
            for (c = 0; c < mNumColumns; ++c, ++i)
            {
                if (mInvMass[i] > 0.0f)
                {
                    mPTmp[i] = mPosition[i] + mStep*mPAllTmp[i].d3;
                    mVTmp[i] = mVelocity[i] + mStep*mVAllTmp[i].d3;
                }
                else
                {
                    mPTmp[i] = mPosition[i];
                    mVTmp[i] = Vector3<float>::Zero();
                }
            }
        }
    }

    // Compute the fourth step.
    for (s = 0, i = 0; s < mNumSlices; ++s)
    {
        for (r = 0; r < mNumRows; ++r)
        {
            for (c = 0; c < mNumColumns; ++c, ++i)
            {
                if (mInvMass[i] > 0.0f)
                {
                    mPAllTmp[i].d4 = mVelocity[i];
                    mVAllTmp[i].d4 = Acceleration(i, c, r, s, fullTime,
                        mPTmp, mVTmp);
                }
            }
        }
    }
    for (s = 0, i = 0; s < mNumSlices; ++s)
    {
        for (r = 0; r < mNumRows; ++r)
        {
            for (c = 0; c < mNumColumns; ++c, ++i)
            {
                if (mInvMass[i] > 0.0f)
                {
                    mPosition[i] += mSixthStep*(mPAllTmp[i].d1 +
                        2.0f*(mPAllTmp[i].d2 + mPAllTmp[i].d3) +
                        mPAllTmp[i].d4);
                    mVelocity[i] += mSixthStep*(mVAllTmp[i].d1 +
                        2.0f*(mVAllTmp[i].d2 + mVAllTmp[i].d3) +
                        mVAllTmp[i].d4);
                }
            }
        }
    }
}

Vector3<float> CpuMassSpringVolume::Acceleration(int32_t i, int32_t c, int32_t r, int32_t s,
    float, std::vector<Vector3<float>> const& position,
    std::vector<Vector3<float>> const& velocity)
{
    // Compute spring forces on position i.  The 'position' and 'velocity'
    // inputs are not necessarily mPosition and mVelocity, because the
    // differential euation solver evaluates the acceleration function at
    // intermediate times.  The face, edge, and corner points of the volume of
    // masses must be handled separately, because each has fewer than eight
    // springs attached to it.

    Vector3<float> diff, force;
    float ratio;
    int32_t prev, next;

    Vector3<float> acceleration = -mViscosity*velocity[i];

    if (c > 0)
    {
        prev = i - 1;  // index to previous column
        diff = position[prev] - position[i];
        ratio = mLengthC[prev] / Length(diff);
        force = mConstantC[prev] * (1.0f - ratio)*diff;
        acceleration += mInvMass[i] * force;
    }

    if (c < mNumColumns - 1)
    {
        next = i + 1;  // index to next column
        diff = position[next] - position[i];
        ratio = mLengthC[i] / Length(diff);
        force = mConstantC[i] * (1.0f - ratio)*diff;
        acceleration += mInvMass[i] * force;
    }

    if (r > 0)
    {
        prev = i - mNumColumns;  // index to previous row
        diff = position[prev] - position[i];
        ratio = mLengthR[prev] / Length(diff);
        force = mConstantR[prev] * (1.0f - ratio)*diff;
        acceleration += mInvMass[i] * force;
    }

    if (r < mNumRows - 1)
    {
        next = i + mNumColumns;  // index to next row
        diff = position[next] - position[i];
        ratio = mLengthR[i] / Length(diff);
        force = mConstantR[i] * (1.0f - ratio)*diff;
        acceleration += mInvMass[i] * force;
    }

    if (s > 0)
    {
        prev = i - mNumSliceElements;  // index to previous slice
        diff = position[prev] - position[i];
        ratio = mLengthS[prev] / Length(diff);
        force = mConstantS[prev] * (1.0f - ratio)*diff;
        acceleration += mInvMass[i] * force;
    }

    if (s < mNumSlices - 1)
    {
        next = i + mNumSliceElements;  // index to next slice
        diff = position[next] - position[i];
        ratio = mLengthS[i] / Length(diff);
        force = mConstantS[i] * (1.0f - ratio)*diff;
        acceleration += mInvMass[i] * force;
    }

    return acceleration;
}

int32_t CpuMassSpringVolume::GetIndex(int32_t c, int32_t r, int32_t s) const
{
    return c + mNumColumns * (r + mNumRows * s);
}

