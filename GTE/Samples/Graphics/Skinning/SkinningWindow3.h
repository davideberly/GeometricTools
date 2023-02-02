// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.28

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/Timer.h>
#include "SkinningEffect.h"
using namespace gte;

class SkinningWindow3 : public Window3
{
public:
    SkinningWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();
    Vector4<float> ComputeWeights(uint32_t a);
    void UpdateConstants(float time);

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
        Vector4<float> weights;
    };

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Visual> mMesh;
    std::shared_ptr<SkinningEffect> mSkinningEffect;
    Timer mSkinningTimer;
};
