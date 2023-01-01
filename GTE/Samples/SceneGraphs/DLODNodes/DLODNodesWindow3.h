// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.02.11

#pragma once

#include <Applications/Window3.h>
#include <Graphics/DLODNode.h>
#include <Graphics/LightEffect.h>
using namespace gte;

class DLODNodesWindow3 : public Window3
{
public:
    DLODNodesWindow3(Parameters& parameters);

    virtual void OnIdle() override;

private:
    void CreateScene();
    void AttachEffect(std::shared_ptr<Visual> const& mesh);
    void UpdateConstants();

    struct VertexPN
    {
        VertexPN()
            :
            position{},
            normal{}
        {
        }

        Vector3<float> position;
        Vector2<float> normal;
    };

    std::shared_ptr<Node> mScene;
    std::shared_ptr<DLODNode> mDLODNode;
    Culler mCuller;
    Vector4<float> mLightWorldDirection;
};
