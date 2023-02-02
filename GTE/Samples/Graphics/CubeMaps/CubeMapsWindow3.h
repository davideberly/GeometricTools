// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/CubeMapEffect.h>
using namespace gte;

class CubeMapsWindow3 : public Window3
{
public:
    CubeMapsWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();

    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mSphere;
    std::shared_ptr<TextureCube> mCubeTexture;
    std::shared_ptr<CubeMapEffect> mCubeMapEffect;
    std::shared_ptr<RasterizerState> mNoCullState;
    Culler mCuller;
};
