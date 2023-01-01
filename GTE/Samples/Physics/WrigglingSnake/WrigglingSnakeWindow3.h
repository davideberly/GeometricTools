// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.07

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/BSplineCurve.h>
#include <Mathematics/Timer.h>
#include "TubeSurface.h"
using namespace gte;

class WrigglingSnakeWindow3 : public Window3
{
public:
    WrigglingSnakeWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateSnakeBody();
    void CreateSnakeHead();
    void UpdateSnake();
    void ModifyCurve();

    // The vertex type for the snake surface.
    struct VertexPT
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // The vertex type for the snake head.
    struct VertexPC
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    // The scene graph.
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<TubeSurface> mSnakeBody;
    std::shared_ptr<Visual> mSnakeSurface;
    std::shared_ptr<Visual> mSnakeHead;

    // The curve and parameters for the snake body.
    size_t mNumCtrlPoints, mDegree;
    float mRadius;
    std::shared_ptr<BSplineCurve<3, float>> mMedial;
    std::shared_ptr<std::function<float(float)>> mRadial;
    size_t mNumMedialSamples;
    size_t mNumSliceSamples;
    std::vector<float> mAmplitudes;
    std::vector<float> mPhases;
    size_t mNumShells;
    std::vector<Vector3<float>> mSlice;

    Timer mMotionTimer;
};
