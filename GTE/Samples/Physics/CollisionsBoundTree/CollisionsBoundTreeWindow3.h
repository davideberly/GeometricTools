// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.20

#pragma once

#include <Applications/Window3.h>
#include <Graphics/BoundingSphere.h>
#include <Graphics/CollisionGroup.h>
#include <Graphics/CollisionMesh.h>
using namespace gte;

class CollisionsBoundTreeWindow3 : public Window3
{
public:
    CollisionsBoundTreeWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    using Bound = BoundingSphere<float>;
    using Mesh = CollisionMesh;
    using CTree = BoundTree<Mesh, Bound>;
    using CRecord = CollisionRecord<Mesh, Bound>;
    using CGroup = CollisionGroup<Mesh, Bound>;

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    void CreateScene();
    bool Transform(uint8_t key);
    void ResetColors();
    void Response(CRecord& record0, int32_t t0, CRecord& record1,
        int32_t t1, float contactTime);

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<Visual> mCylinder0, mCylinder1;
    std::shared_ptr<Mesh> mCylinderMesh0, mCylinderMesh1;
    std::shared_ptr<CGroup> mGroup;
    Vector2<float> mBlueUV, mRedUV, mCyanUV, mYellowUV;
};
