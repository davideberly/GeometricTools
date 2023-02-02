// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BloodCellController.h"
using namespace gte;

BloodCellController::BloodCellController(std::shared_ptr<Camera> const& camera, BufferUpdater const& postUpdate)
    :
    ParticleController(camera, postUpdate),
    mURD(-0.01f, 0.01f)
{
}

void BloodCellController::UpdatePointMotion(float)
{
    Particles* particles = reinterpret_cast<Particles*>(mObject);
    auto& posSize = particles->GetPositionSize();
    uint32_t numActive = particles->GetNumActive();
    float const maxSize = 0.25f;
    for (uint32_t i = 0; i < numActive; ++i)
    {
        for (uint32_t j = 0; j < 3; ++j)
        {
            posSize[i][j] += mURD(mDRE);
            if (posSize[i][j] > 1.0f)
            {
                posSize[i][j] = 1.0f;
            }
            else if (posSize[i][j] < -1.0f)
            {
                posSize[i][j] = -1.0f;
            }
        }

        posSize[i][3] *= (1.0f + mURD(mDRE));
        if (posSize[i][3] > maxSize)
        {
            posSize[i][3] = maxSize;
        }
    }

    particles->GenerateParticles(mCamera);
    mPostUpdate(particles->GetVertexBuffer());
}
