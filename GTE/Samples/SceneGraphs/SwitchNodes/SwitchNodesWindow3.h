// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window3.h>
#include <Graphics/SwitchNode.h>
using namespace gte;

class SwitchNodesWindow3 : public Window3
{
public:
    SwitchNodesWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void AttachEffect(std::shared_ptr<Visual> const& mesh,
        std::shared_ptr<Texture2> const& texture);

    struct VertexPT
    {
        VertexPT()
            :
            position{},
            tcoord{}
        {
        }

        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::shared_ptr<Node> mScene;
    std::shared_ptr<SwitchNode> mSwitchNode;
    Culler mCuller;
};


