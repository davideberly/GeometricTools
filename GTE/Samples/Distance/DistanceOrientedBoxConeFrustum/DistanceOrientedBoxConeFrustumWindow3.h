// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/DistOrientedBox3Cone3.h>
#include "DistanceBoxQuad.h"
#include <random>
using namespace gte;

class DistanceOrientedBoxConeFrustumWindow3 : public Window3
{
public:
    DistanceOrientedBoxConeFrustumWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();
    void CreateBoxMesh();
    void CreateConeMesh();
    void CreateQuadMesh();
    void CreateSegmentMeshes();
    void CreateClosestPointMeshes();
    void Translate(int32_t direction, float delta);
    void Rotate(int32_t direction, float delta);
    void Update();

    struct Vertex
    {
        Vector<3, float> position;
        Vector<4, float> color;
    };

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<Visual> mBoxMesh;
    std::shared_ptr<Visual> mConeMesh;
    std::shared_ptr<Visual> mQuadMesh;
    std::shared_ptr<Visual> mBoxQuadSegmentMesh;
    std::shared_ptr<Visual> mBoxConeSegmentMesh;
    std::shared_ptr<Visual> mBoxClosestToQuadMesh;
    std::shared_ptr<Visual> mBoxClosestToConeMesh;
    std::shared_ptr<Visual> mQuadClosestMesh;
    std::shared_ptr<Visual> mConeClosestMesh;
    VertexFormat mVFormat;

    std::default_random_engine mDRE;
    std::uniform_real_distribution<float> mURD;

    OrientedBox<3, float> mBox;
    Cone<3, float> mCone;
    std::array<Vector<3, float>, 4> mQuadrilateral;
    float mQuadAngle;
    float mBoxQuadDistance;
    float mBoxConeDistance;
    Vector<3, float> mBoxClosestToQuad;
    Vector<3, float> mBoxClosestToCone;
    Vector<3, float> mQuadClosest;
    Vector<3, float> mConeClosest;
    DistanceBoxQuad<float> mBoxQuadQuery;
    DCPQuery<float, OrientedBox<3, float>, Cone<3, float>> mBoxConeQuery;
};
