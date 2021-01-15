// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
#include <Applications/Timer.h>
#include <Graphics/DirectionalLightEffect.h>
#include <Mathematics/BoxManager.h>
#include <random>
using namespace gte;

class IntersectingBoxesWindow3 : public Window3
{
public:
    IntersectingBoxesWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void ModifyBoxes();
    void ModifyMesh(int i);
    void PhysicsTick();
    void GraphicsTick();

    std::vector<AlignedBox3<float>> mBoxes;
    std::unique_ptr<BoxManager<float>> mManager;
    bool mDoSimulation;
    Timer mSimulationTimer;
    double mLastIdle;
    float mSize;

    enum { NUM_BOXES = 16 };

    struct Vertex
    {
        Vector3<float> position, normal;
    };

    std::shared_ptr<Node> mScene;
    std::shared_ptr<RasterizerState> mWireState;
    std::mt19937 mMTE;
    std::uniform_real_distribution<float> mPerturb;
    std::array<std::shared_ptr<Visual>, NUM_BOXES> mBoxMesh;
    std::array<std::shared_ptr<DirectionalLightEffect>, NUM_BOXES> mNoIntersectEffect;
    std::array<std::shared_ptr<DirectionalLightEffect>, NUM_BOXES> mIntersectEffect;
};
