// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/ConstantColorEffect.h>
#include <Graphics/VertexColorEffect.h>
#include <Mathematics/Hyperplane.h>
using namespace gte;

class SplitMeshByPlaneWindow3 : public Window3
{
public:
    SplitMeshByPlaneWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;
    virtual bool OnMouseMotion(MouseButton button, int32_t x, int32_t y, uint32_t modifiers) override;

private:
    void CreateScene();
    void Update();

    struct TorusVertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    // The application shows a torus split by a plane.  You can rotate the
    // orus using the virtual trackball to see how the mesh is partitioned.
    // The first array stores the torus vertices in model-space coordinates
    // and the second array stores the vertices transformed to world space.
    std::vector<Vector3<float>> mTorusVerticesMS;
    std::vector<Vector3<float>> mTorusVerticesWS;
    std::vector<int32_t> mTorusIndices;
    Plane3<float> mPlane;

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Visual> mMeshTorus;
    std::shared_ptr<Visual> mMeshPlane;
    std::shared_ptr<VertexColorEffect> mTorusEffect;
    std::shared_ptr<ConstantColorEffect> mMeshEffect;

    bool mTorusMoved;
};
