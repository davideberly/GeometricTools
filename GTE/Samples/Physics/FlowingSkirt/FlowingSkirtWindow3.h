// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Applications/Timer.h>
#include <Mathematics/BSplineCurve.h>
using namespace gte;

class FlowingSkirtWindow3 : public Window3
{
public:
    FlowingSkirtWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void UpdateSkirt();
    void ModifyCurves();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mSkirt;
    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mWireNoCullState;

    // The skirt is a generalized Bezier cylinder.
    int32_t mNumCtrl, mDegree;
    float mATop, mBTop, mABottom, mBBottom;
    std::unique_ptr<BSplineCurve<3, float>> mSkirtTop;
    std::unique_ptr<BSplineCurve<3, float>> mSkirtBottom;
    std::vector<float> mFrequencies;
    Timer mAnimTimer;
};
