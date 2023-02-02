// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.02.24

#pragma once

#include <Applications/Window3.h>
#include <Graphics/CLODMesh.h>
using namespace gte;

class CLODMeshesWindow3 : public Window3
{
public:
    CLODMeshesWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void UpdateCLODMesh();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;

        inline Vector3<float> GetPosition() const
        {
            return position;
        }
    };

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Node> mScene, mTrnNode;
    std::array<std::shared_ptr<CLODMesh>, 2> mCLODMesh;
    std::array<float, 4> mTextColor;
};
