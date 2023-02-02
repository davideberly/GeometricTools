// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
using namespace gte;

class PlaneEstimationWindow2 : public Window2
{
public:
    PlaneEstimationWindow2(Parameters& parameters);

    virtual void OnDisplay() override;

private:
    bool SetEnvironment();
    std::shared_ptr<ConstantBuffer> CreateBezierControls();

    std::string mShaderSourceEvaluateBezier;
    std::string mShaderSourcePlaneEstimation;
    std::string mShaderSourcePlaneVisualize;
    std::string mShaderSourcePositionVisualize;
    std::shared_ptr<Texture2> mPositions;
    std::shared_ptr<Texture2> mPlanes;
    std::shared_ptr<ComputeProgram> mPositionProgram;
    std::shared_ptr<ComputeProgram> mPlaneProgram;
    uint32_t mNumXGroups, mNumYGroups;
    std::shared_ptr<OverlayEffect> mOverlay[2];
};
