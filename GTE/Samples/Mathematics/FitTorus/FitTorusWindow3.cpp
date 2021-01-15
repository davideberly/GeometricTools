// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "FitTorusWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>
#include <Mathematics/ApprTorus3.h>
#include <random>

FitTorusWindow3::FitTorusWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mTextColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    mNoCullSolidState = std::make_shared<RasterizerState>();
    mNoCullSolidState->cullMode = RasterizerState::CULL_NONE;
    mNoCullSolidState->fillMode = RasterizerState::FILL_SOLID;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState->fillMode = RasterizerState::FILL_WIREFRAME;
    mEngine->SetRasterizerState(mNoCullSolidState);

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;

    CreateScene();

    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.005f, 0.002f,
        { -6.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void FitTorusWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    mEngine->Draw(mPoints);

    mEngine->SetBlendState(mBlendState);
    if (mGNTorus->culling == CullingMode::CULL_NEVER)
    {
        mEngine->Draw(mGNTorus);
    }
    if (mLMTorus->culling == CullingMode::CULL_NEVER)
    {
        mEngine->Draw(mLMTorus);
    }
    mEngine->SetDefaultBlendState();

    mEngine->Draw(8, 24, mTextColor, "key '0' toggles GN-generated mesh");
    mEngine->Draw(8, 48, mTextColor, "key '1' toggles LM-generated mesh");
    mEngine->Draw(8, 72, mTextColor, "key 'w' toggles wireframe");
    mEngine->Draw(8, mYSize - 8, mTextColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool FitTorusWindow3::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mNoCullSolidState)
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullSolidState);
        }
        return true;
    case '0':
        if (mGNTorus->culling == CullingMode::CULL_NEVER)
        {
            mGNTorus->culling = CullingMode::CULL_ALWAYS;
        }
        else
        {
            mGNTorus->culling = CullingMode::CULL_NEVER;
        }
        return true;
    case '1':
        if (mLMTorus->culling == CullingMode::CULL_NEVER)
        {
            mLMTorus->culling = CullingMode::CULL_ALWAYS;
        }
        else
        {
            mLMTorus->culling = CullingMode::CULL_NEVER;
        }
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

void FitTorusWindow3::CreateScene()
{
    std::default_random_engine dre;
    std::uniform_real_distribution<double> rnd(-1.0, 1.0);
    double const epsilon = 0.01;

    Vector3<double> C{ 0.0, 0.0, 0.0 };
    Vector3<double> N{ 0.0, 0.0, 1.0 };
    Vector3<double> D0{ 1.0, 0.0, 0.0 };
    Vector3<double> D1{ 0.0, 1.0, 0.0 };
    double r0 = 1.0;
    double r1 = 0.25;

    unsigned int const numPoints = 1024;
    std::vector<Vector3<double>> X(numPoints);
    for (unsigned int i = 0; i < numPoints; ++i)
    {
        double angle0 = GTE_C_PI * rnd(dre);  // in [-pi,pi)
        double angle1 = GTE_C_PI * rnd(dre);  // in [-pi,pi)
        double cs0 = cos(angle0);
        double sn0 = sin(angle0);
        double cs1 = cos(angle1);
        double sn1 = sin(angle1);
        double pr0 = (1.0 + epsilon * rnd(dre)) * r0;  // in [(1-e)*r0, (1+e)*r0)
        double pr1 = (1.0 + epsilon * rnd(dre)) * r1;  // in [(1-e)*r1, (1+e)*r1)
        X[i] = C + (pr0 + pr1 * cs1) * (cs0 * D0 + sn0 * D1) + pr1 * sn1 * N;
    }

    CreatePoints(X);

    Vector3<double> torusC, torusN;
    double torusR0, torusR1;

    CreateGNTorus(X, torusC, torusN, torusR0, torusR1);
    Vector4<float> green{ 0.0f, 1.0f, 0.0f, 0.25f };
    mGNTorus = CreateTorusMesh(torusC, torusN, torusR0, torusR1, green);

    CreateLMTorus(X, torusC, torusN, torusR0, torusR1);
    Vector4<float> blue{ 0.0f, 0.0f, 1.0f, 0.25f };
    mLMTorus = CreateTorusMesh(torusC, torusN, torusR0, torusR1, blue);
}

void FitTorusWindow3::CreatePoints(std::vector<Vector3<double>> const& X)
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    unsigned int numVertices = static_cast<unsigned int>(X.size());
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    auto vertices = vbuffer->Get<Vector3<float>>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            vertices[i][j] = static_cast<float>(X[i][j]);
        }
    }

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYPOINT, numVertices);

    Vector4<float> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, black);

    mPoints = std::make_shared<Visual>(vbuffer, ibuffer, effect);

    mPVWMatrices.Subscribe(mPoints->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mPoints);
}

void FitTorusWindow3::CreateGNTorus(std::vector<Vector3<double>> const& X,
    Vector3<double>& C, Vector3<double>& N, double& r0, double& r1)
{
    ApprTorus3<double> fitter;
    size_t const maxIterations = 128;
    double const updateLengthTolerance = 1e-04;
    double const errorDifferenceTolerance = 1e-08;
    bool useTorusInputAsInitialGuess = false;

    auto result = fitter(static_cast<int>(X.size()), X.data(),
        maxIterations, updateLengthTolerance, errorDifferenceTolerance,
        useTorusInputAsInitialGuess, C, N, r0, r1);
    (void)result;
}

void FitTorusWindow3::CreateLMTorus(std::vector<Vector3<double>> const& X,
    Vector3<double>& C, Vector3<double>& N, double& r0, double& r1)
{
    ApprTorus3<double> fitter;
    size_t const maxIterations = 128;
    double const updateLengthTolerance = 1e-04;
    double const errorDifferenceTolerance = 1e-08;
    double const lambdaFactor = 0.001;
    double const lambdaAdjust = 10.0;
    size_t const maxAdjustments = 8;
    bool useTorusInputAsInitialGuess = false;

    auto result = fitter(static_cast<int>(X.size()), X.data(),
        maxIterations, updateLengthTolerance, errorDifferenceTolerance,
        lambdaFactor, lambdaAdjust, maxAdjustments, useTorusInputAsInitialGuess,
        C, N, r0, r1);
    (void)result;
}

std::shared_ptr<Visual> FitTorusWindow3::CreateTorusMesh(
    Vector3<double> const& C, Vector3<double> const& N, double r0, double r1,
    Vector4<float> const& color)
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    unsigned int const numCircleSamples = 16;
    unsigned int const numRadialSamples = 16;
    auto torus = mf.CreateTorus(numCircleSamples, numRadialSamples,
        static_cast<float>(r0), static_cast<float>(r1));
    torus->culling = CullingMode::CULL_ALWAYS;

    Vector3<float> center;
    for (int j = 0; j < 3; ++j)
    {
        center[j] = static_cast<float>(C[j]);
    }
    torus->localTransform.SetTranslation(-center);

    Vector3<float> basis[3];
    for (int j = 0; j < 3; ++j)
    {
        basis[0][j] = static_cast<float>(N[j]);
    }
    ComputeOrthogonalComplement(1, basis);
    Matrix3x3<float> rotate;
    rotate.SetCol(0, basis[1]);
    rotate.SetCol(1, basis[2]);
    rotate.SetCol(2, basis[0]);
    torus->localTransform.SetRotation(rotate);

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    torus->SetEffect(effect);

    mPVWMatrices.Subscribe(torus->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(torus);

    return torus;
}
