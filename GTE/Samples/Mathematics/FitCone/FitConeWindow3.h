// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
using namespace gte;

class FitConeWindow3 : public Window3
{
public:
    FitConeWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();
    void CreatePoints(std::vector<Vector3<double>> const& X);

    void CreateGNCone(std::vector<Vector3<double>> const& X,
        Vector3<double>& coneVertex, Vector3<double>& coneAxis, double& coneAngle);

    void CreateLMCone(std::vector<Vector3<double>> const& X,
        Vector3<double>& coneVertex, Vector3<double>& coneAxis, double& coneAngle);

    std::shared_ptr<Visual> CreateConeMesh(std::vector<Vector3<double>> const& X,
        Vector3<double> const& coneVertex, Vector3<double> const& coneAxis,
        double coneAngle, Vector4<float> const& color);

    std::shared_ptr<RasterizerState> mNoCullSolidState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<Visual> mPoints;
    std::shared_ptr<Visual> mGNCone;
    std::shared_ptr<Visual> mLMCone;
    std::array<float, 4> mTextColor;
    Vector3<float> mCenter;
};
