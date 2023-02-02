// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
using namespace gte;

#define USE_DRAW_DIRECT
//#define SAVE_RENDERING_TO_DISK

class GeometryShadersWindow3 : public Window3
{
public:
    GeometryShadersWindow3(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    bool CreateScene();
    void UpdateConstants();

    std::shared_ptr<ConstantBuffer> mMatrices;
    std::shared_ptr<Visual> mMesh;

#if !defined(USE_DRAW_DIRECT)
    std::shared_ptr<StructuredBuffer> mParticles;
#endif

#if defined(SAVE_RENDERING_TO_DISK)
    std::shared_ptr<DrawTarget> mTarget;
#endif
};
