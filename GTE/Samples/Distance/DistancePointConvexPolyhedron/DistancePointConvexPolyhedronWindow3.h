// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
#include <Graphics/ConstantColorEffect.h>
#include <Mathematics/DistPoint3ConvexPolyhedron3.h>
using namespace gte;

class DistancePointConvexPolyhedronWindow3 : public Window3
{
public:
    DistancePointConvexPolyhedronWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void Translate(int direction, float delta);
    void Rotate(int direction, float delta);
    void DoQuery();

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<Visual> mPointMesh, mPolyhedronMesh;
    std::shared_ptr<ConstantColorEffect> mRedEffect, mBlueEffect;
    std::shared_ptr<Visual> mSegment;
    Vector3<float> mPoint;
    ConvexPolyhedron3<float> mPolyhedron;
    Vector3<float> mPolyhedronCenter;
    DCPQuery<float, Vector3<float>, ConvexPolyhedron3<float>> mQuery;
};
