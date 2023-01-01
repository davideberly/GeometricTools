// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
using namespace gte;

//#define DO_CPU_MASS_SPRING
#if defined(DO_CPU_MASS_SPRING)
#include "CpuMassSpringVolume.h"
typedef CpuMassSpringVolume MassSpringSystem;
#else
#include "GpuMassSpringVolume.h"
typedef GpuMassSpringVolume MassSpringSystem;
#endif

class MassSprings3DWindow3 : public Window3
{
public:
    MassSprings3DWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    bool CreateMassSpringSystem();
    bool CreateBoxFaces();
    void UpdateTransforms();
    void UpdateMassSpringSystem();

    inline int32_t GetIndex(int32_t x, int32_t y, int32_t z) const
    {
        return x + mDimension[0] * (y + mDimension[1] * z);
    }

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<VertexBuffer> mVBuffer;
    std::shared_ptr<Visual> mBoxFace[6];
    std::shared_ptr<VisualEffect> mEffect[6];
    std::unique_ptr<MassSpringSystem> mMassSprings;
    float mSimulationTime, mSimulationDelta;
    int32_t mDimension[3];
};
