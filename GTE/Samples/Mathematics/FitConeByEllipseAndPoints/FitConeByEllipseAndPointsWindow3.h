// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.02.25

#pragma once

#include <Applications/Window3.h>
using namespace gte;

class FitConeByEllipseAndPointsWindow3 : public Window3
{
public:
    FitConeByEllipseAndPointsWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void DeleteScene();

    static std::array<std::string, 4> msFile;
    size_t mFileSelection;

    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;

    std::vector<Vector3<double>> mPoints;
    std::shared_ptr<Visual> mPointMesh;
    std::vector<std::shared_ptr<Visual>> mBoxMesh;
    std::array<std::shared_ptr<Visual>, 2> mEllipseMesh;
    std::shared_ptr<Visual> mConeMesh;

    bool mDrawPointMesh;
    bool mDrawBoxMesh;
    bool mDrawEllipseMesh;
    bool mDrawConeMesh;
};
