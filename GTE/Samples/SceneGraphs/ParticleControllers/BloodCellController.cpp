// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

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
    unsigned int numActive = particles->GetNumActive();
    float const maxSize = 0.25f;
    for (unsigned int i = 0; i < numActive; ++i)
    {
        for (unsigned int j = 0; j < 3; ++j)
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
