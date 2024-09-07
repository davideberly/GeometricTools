// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 7.2.2024.09.06

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/ApprEllipsoid3.h>
using namespace gte;

class ApproximateEllipsoid3Window3 : public Window3
{
public:
    ApproximateEllipsoid3Window3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();
    std::shared_ptr<Visual> CreateEllipsoidMesh(Ellipsoid3<float> const& ellipsoid,
        Vector4<float> const& color);

    ApprEllipsoid3<float> mFitter;
    Ellipsoid3<float> mTrueEllipsoid, mApprEllipsoid;

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Visual> mTrueMesh, mApprMesh;
};
