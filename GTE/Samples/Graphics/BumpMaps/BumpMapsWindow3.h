// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
#include <Graphics/BumpMapEffect.h>
using namespace gte;

class BumpMapsWindow3 : public Window3
{
public:
    BumpMapsWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseMotion(MouseButton button, int x, int y, unsigned int modifiers) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateBumpMappedTorus();
    void CreateTexturedTorus();
    void UpdateBumpMap();

    std::shared_ptr<Node> mScene;
    std::shared_ptr<BumpMapEffect> mBumpMapEffect;
    std::shared_ptr<Visual> mBumpMappedTorus;
    std::shared_ptr<Visual> mTexturedTorus;
    Vector4<float> mLightDirection;
    bool mUseBumpMap;
};
