// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/BSplineCurveFit.h>
using namespace gte;

class BSplineCurveFitterWindow3 : public Window3
{
public:
    BSplineCurveFitterWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void CreateBSplinePolyline();

    enum { NUM_SAMPLES = 1000 };

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    std::vector<Vector3<float>> mSamples;
    std::shared_ptr<Visual> mHelix, mPolyline;

    int mDegree, mNumControls;
    std::unique_ptr<BSplineCurveFit<float>> mSpline;
    float mAvrError, mRmsError;
    std::string mMessage;
};
