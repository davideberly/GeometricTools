// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.22

#pragma once

#include <Applications/Window3.h>
using namespace gte;

class VolumeFogWindow3 : public Window3
{
public:
    VolumeFogWindow3(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateBackground();
    void CreateMesh();
    void UpdateFog();

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
        Vector2<float> tcoord;
    };

    std::shared_ptr<DepthStencilState> mNoDepthStencilState;
    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Visual> mMesh;
};
