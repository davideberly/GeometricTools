// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
using namespace gte;

class FitTorusWindow3 : public Window3
{
public:
    FitTorusWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();
    void CreatePoints(std::vector<Vector3<double>> const& X);

    void CreateGNTorus(std::vector<Vector3<double>> const& X,
        Vector3<double>& C, Vector3<double>& N, double& r0, double& r1);

    void CreateLMTorus(std::vector<Vector3<double>> const& X,
        Vector3<double>& C, Vector3<double>& N, double& r0, double& r1);

    std::shared_ptr<Visual> CreateTorusMesh(Vector3<double> const& C,
        Vector3<double> const& N, double r0, double r1, Vector4<float> const& color);

    std::shared_ptr<RasterizerState> mNoCullSolidState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<Visual> mPoints;
    std::shared_ptr<Visual> mGNTorus;
    std::shared_ptr<Visual> mLMTorus;
    std::array<float, 4> mTextColor;
};
