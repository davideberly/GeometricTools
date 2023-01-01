// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.28

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/IntrLine3Torus3.h>
using namespace gte;

class IntersectLineTorusWindow3 : public Window3
{
public:
    IntersectLineTorusWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;
    virtual bool OnMouseClick(MouseButton button, MouseState state,
        int32_t x, int32_t y, uint32_t modifiers) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateLine();
    void CreateTorus();
    void CreateSpheres();
    void DoPick(int32_t x, int32_t y);
    void Update();

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<Visual> mLineMesh;
    std::shared_ptr<Visual> mTorusMesh;
    std::array<std::shared_ptr<Visual>, 4> mSphereMesh;
    float mLineExtent;

    Line3<double> mLine;
    Torus3<double> mTorus;
    FIQuery<double, Line3<double>, Torus3<double>> mQuery;
    FIQuery<double, Line3<double>, Torus3<double>>::Result mResult;
    bool mUseLinePoints;
};
