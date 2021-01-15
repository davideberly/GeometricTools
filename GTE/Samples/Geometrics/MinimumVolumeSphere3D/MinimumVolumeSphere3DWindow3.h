// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/MinimumVolumeSphere3.h>
using namespace gte;

class MinimumVolumeSphere3DWindow3 : public Window3
{
public:
    MinimumVolumeSphere3DWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void UpdateScene();

    enum { NUM_POINTS = 256 };
    std::array<std::shared_ptr<Visual>, NUM_POINTS> mPoints;
    std::shared_ptr<Visual> mSegments, mSphere;
    std::shared_ptr<RasterizerState> mNoCullWireState;

    int mNumActive;
    std::vector<Vector3<float>> mVertices;
    Sphere3<float> mMinimalSphere;
    MinimumVolumeSphere3<float, BSRational<UIntegerAP32>> mMVS3;
};
