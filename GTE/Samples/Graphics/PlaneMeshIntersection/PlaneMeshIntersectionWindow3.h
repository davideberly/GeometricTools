// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
using namespace gte;

class PlaneMeshIntersectionWindow3 : public Window3
{
public:
    PlaneMeshIntersectionWindow3(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    bool CreateScene();
    void UpdateMatrices();

    struct Vertex
    {
        Vector3<float> position, color;
    };

    struct PMIParameters
    {
        Matrix4x4<float> pvMatrix;
        Matrix4x4<float> wMatrix;
        Vector4<float> planeVector0;
        Vector4<float> planeVector1;
    };

    std::shared_ptr<ConstantBuffer> mPMIParameters;
    std::shared_ptr<Visual> mMesh;

    std::shared_ptr<DrawTarget> mPSTarget;
    std::shared_ptr<TextureRT> mPSColor;
    std::shared_ptr<TextureRT> mPSPlaneConstant;
    std::shared_ptr<Texture2> mScreen;
    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<ComputeProgram> mDrawIntersections;
};
