// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Environment.h>
#include <Graphics/Graphics.h>
using namespace gte;

class GpuMassSpringVolume
{
public:
    // Construction and destruction.  This class represents a CxRxS array of
    // masses lying on in a volume and connected by an array of springs.  The
    // masses are indexed by mass(c,r,s) for 0 <= c < C, 0 <= r < R, and
    // 0 <= s < S.  The mass at interior position X(c,r,s) is connected by
    // springs to the masses at positions X(c,r-1,s), X(c,r+1,s), X(c-1,r,s),
    // X(c+1,r,s), X(c,r,s-1), and X(c,r,s+1).  Boundary masses have springs
    // connecting them to their neighbors: a "face" mass has 5 neighbors, an
    // "edge" mass has 4 neighbors, and a "corner" mass has 3 neighbors.
    ~GpuMassSpringVolume() = default;
    GpuMassSpringVolume(std::shared_ptr<GraphicsEngine> const& engine,
        std::shared_ptr<ProgramFactory> const& factory, int32_t numColumns,
        int32_t numRows, int32_t numSlices, float step, float viscosity,
        Environment& environment, bool& created);

    // Deferred construction.  The physical parameters must be set before
    // starting the simulation.

    // Basic physical parameters.  The indices must satisfy 0 <= c < C,
    // 0 <= r < R, and 0 <= s < S.
    void SetMass(int32_t c, int32_t r, int32_t s, float mass);
    void SetPosition(int32_t c, int32_t r, int32_t s, Vector3<float> const& position);
    void SetVelocity(int32_t c, int32_t r, int32_t s, Vector3<float> const& velocity);

    // Each interior mass at (c,r,s) has 6 adjacent springs.  Face masses
    // have only 5 neighbors, edge masses have only 4 neighbors, and corner
    // masses have only 3 neighbors.  Each mass provides access to 3 adjacent
    // springs at (c+1,r,s), (c,r+1,s), and (c,r,s+1).  The face, edge, and
    // corner masses provide access to only an appropriate subset of these.
    // The indices must satisfy
    //   ConstantC/LengthC:  0 <= c < C-1, 0 <= r < R,   0 <= s < S
    //   ConstantR/LengthR:  0 <= c < C,   0 <= r < R-1, 0 <= s < S
    //   ConstantS/LengthS:  0 <= c < C,   0 <= r < R,   0 <= s < S-1
    void SetConstantC(int32_t c, int32_t r, int32_t s, float v);  // spring to (c+1,r,s)
    void SetLengthC(int32_t c, int32_t r, int32_t s, float v);    // spring to (c+1,r,s)
    void SetConstantR(int32_t c, int32_t r, int32_t s, float v);  // spring to (c,r+1,s)
    void SetLengthR(int32_t c, int32_t r, int32_t s, float v);    // spring to (c,r+1,s)
    void SetConstantS(int32_t c, int32_t r, int32_t s, float v);  // spring to (c,r,s+1)
    void SetLengthS(int32_t c, int32_t r, int32_t s, float v);    // spring to (c,r,s+1)

    // Member access.
    Vector3<float> GetPosition(int32_t c, int32_t r, int32_t s) const;
    std::shared_ptr<StructuredBuffer>& GetPosition();

    // Update the particle positions and velocities based on current time and
    // particle state.
    void Update(float time, std::shared_ptr<GraphicsEngine> const& engine);

private:
    inline int32_t GetIndex(int32_t c, int32_t r, int32_t s) const
    {
        return c + mNumColumns * (r + mNumRows * s);
    }

    struct SimulationParameters
    {
        int32_t dimensions[4];
        float viscosity;
        float time;
        float delta;
        float halfDelta;
        float sixthDelta;
        float halfTime;
        float fullTime;
    };
    std::shared_ptr<ConstantBuffer> mParameters;

    int32_t mNumColumns, mNumRows;
    std::shared_ptr<StructuredBuffer> mMass, mInvMass;
    std::shared_ptr<StructuredBuffer> mPosition, mVelocity;
    std::shared_ptr<StructuredBuffer> mConstantC, mLengthC;
    std::shared_ptr<StructuredBuffer> mConstantR, mLengthR;
    std::shared_ptr<StructuredBuffer> mConstantS, mLengthS;
    std::shared_ptr<StructuredBuffer> mPTmp, mPAllTmp;
    std::shared_ptr<StructuredBuffer> mVTmp, mVAllTmp;

    uint32_t mNumXGroups, mNumYGroups, mNumZGroups;
    std::shared_ptr<ComputeProgram> mRK4Shader[8];
};
