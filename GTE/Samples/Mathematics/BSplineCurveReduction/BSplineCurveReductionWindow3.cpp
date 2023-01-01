// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.06.08

#include "BSplineCurveReductionWindow3.h"
#include <Graphics/ConstantColorEffect.h>
#include <Mathematics/BSplineCurve.h>
#include <Mathematics/BSplineReduction.h>

BSplineCurveReductionWindow3::BSplineCurveReductionWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 10000.0f, 1.0f, 0.01f,
        { 0.0f, 0.0f, -600.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
    
    CreateScene();
    mTrackBall.Update();
    mPVWMatrices.Update();
}

void BSplineCurveReductionWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mOriginal);
    mEngine->Draw(mReduced);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool BSplineCurveReductionWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Mathematics/BSplineCurveReduction/Data");

    if (mEnvironment.GetPath("ControlPoints.txt") == "")
    {
        LogError("Cannot find file ControlPoints.txt");
        return false;
    }
    return true;
}

void BSplineCurveReductionWindow3::CreateScene()
{
    // Keep track of the average to translate the two curves to the
    // origin for trackball rotation.
    Vector3<double> daverage = { 0.0, 0.0, 0.0 };

    // Load the control points for the B-spline curve.
    std::string path = mEnvironment.GetPath("ControlPoints.txt");
    std::ifstream inFile(path);
    int32_t numInControls;
    inFile >> numInControls;
    std::vector<Vector3<double>> inControls(numInControls);
    for (int32_t i = 0; i < numInControls; ++i)
    {
        inFile >> inControls[i][0];
        inFile >> inControls[i][1];
        inFile >> inControls[i][2];
        daverage += inControls[i];
    }
    inFile.close();

    // Create the B-spline curve.
    int32_t const degree = 3;
    BasisFunctionInput<double> inBasis(numInControls, degree);
    BSplineCurve<3, double> original(inBasis, inControls.data());

    // Create the control points for the reduced B-spline curve.
    std::vector<Vector3<double>> outControls;
    double fraction = 0.1;
    BSplineReduction<3, double> reducer;
    reducer(inControls, degree, fraction, outControls);
    int32_t numOutControls = static_cast<int32_t>(outControls.size());
    BasisFunctionInput<double> outBasis(numOutControls, degree);
    BSplineCurve<3, double> reduced(outBasis, outControls.data());
    for (int32_t i = 0; i < numOutControls; ++i)
    {
        daverage += outControls[i];
    }
    int32_t totalControls = numInControls + numOutControls;
    daverage /= static_cast<double>(totalControls);
    Vector3<float> average
    {
        static_cast<float>(daverage[0]),
        static_cast<float>(daverage[1]),
        static_cast<float>(daverage[2])
    };

    // The vertex format is shared by the Visual objects for both curves.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);

    // Create the polyline approximation to visualize the original curve.
    uint32_t numVertices = static_cast<uint32_t>(numInControls);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    auto vertices = vbuffer->Get<Vector3<float>>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        double t = i / static_cast<double>(numVertices);
        std::array<Vector3<double>, 4> jet{};
        original.Evaluate(t, 0, jet.data());
        for (int32_t j = 0; j < 3; ++j)
        {
            vertices[i][j] = static_cast<float>(jet[0][j]);
        }
    }

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_CONTIGUOUS, numVertices - 1);

    Vector4<float> red = { 1.0f, 0.0f, 0.0f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, red);

    mOriginal = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mOriginal->localTransform.SetTranslation(-average);
    mPVWMatrices.Subscribe(mOriginal->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mOriginal);

    // Create the polyline approximation to visualize the reduced curve.
    numVertices = static_cast<uint32_t>(outControls.size());
    vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vertices = vbuffer->Get<Vector3<float>>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        double t = i / static_cast<double>(numVertices);
        std::array<Vector3<double>, 4> jet{};
        reduced.Evaluate(t, 0, jet.data());
        for (int32_t j = 0; j < 3; ++j)
        {
            vertices[i][j] = static_cast<float>(jet[0][j]);
        }
    }

    ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_CONTIGUOUS, numVertices - 1);

    Vector4<float> blue = { 0.0f, 0.0f, 1.0f, 1.0f };
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, blue);

    mReduced = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mReduced->localTransform.SetTranslation(-average);
    mPVWMatrices.Subscribe(mReduced->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mReduced);
}
