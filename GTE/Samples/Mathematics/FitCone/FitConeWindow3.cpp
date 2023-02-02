// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "FitConeWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>
#include <Mathematics/ApprCone3.h>
#include <random>

FitConeWindow3::FitConeWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mTextColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    mNoCullSolidState = std::make_shared<RasterizerState>();
    mNoCullSolidState->cull = RasterizerState::Cull::NONE;
    mNoCullSolidState->fill = RasterizerState::Fill::SOLID;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetRasterizerState(mNoCullSolidState);

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;

    CreateScene();

    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.005f, 0.002f,
        { -6.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void FitConeWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    mEngine->Draw(mPoints);

    mEngine->SetBlendState(mBlendState);
    if (mGNCone->culling == CullingMode::NEVER)
    {
        mEngine->Draw(mGNCone);
    }
    if (mLMCone->culling == CullingMode::NEVER)
    {
        mEngine->Draw(mLMCone);
    }
    mEngine->SetDefaultBlendState();

    mEngine->Draw(8, 24, mTextColor, "key '0' toggles GN-generated mesh");
    mEngine->Draw(8, 48, mTextColor, "key '1' toggles LM-generated mesh");
    mEngine->Draw(8, 72, mTextColor, "key 'w' toggles wireframe");
    mEngine->Draw(8, mYSize - 8, mTextColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool FitConeWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
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
        if (mGNCone->culling == CullingMode::NEVER)
        {
            mGNCone->culling = CullingMode::ALWAYS;
        }
        else
        {
            mGNCone->culling = CullingMode::NEVER;
        }
        return true;
    case '1':
        if (mLMCone->culling == CullingMode::NEVER)
        {
            mLMCone->culling = CullingMode::ALWAYS;
        }
        else
        {
            mLMCone->culling = CullingMode::NEVER;
        }
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

void FitConeWindow3::CreateScene()
{
    std::default_random_engine dre;
    std::uniform_real_distribution<double> rnd(-1.0, 1.0);
    double const epsilon = 0.01;

    Vector3<double> V = { 3.0, 2.0, 1.0 };
    Vector3<double> U = { 1.0, 2.0, 3.0 };
    std::array<Vector3<double>, 3> basis{};
    basis[0] = U;
    ComputeOrthogonalComplement(1, basis.data());
    U = basis[0];
    Vector3<double> W0 = basis[1];
    Vector3<double> W1 = basis[2];
    double H0 = 1.0;
    double H1 = 2.0;
    double theta = GTE_C_PI / 4.0;
    double tantheta = std::tan(theta);

    uint32_t const numPoints = 8196;
    std::vector<Vector3<double>> X(numPoints);
    for (uint32_t i = 0; i < numPoints; ++i)
    {
        double unit = 0.5 * (rnd(dre) + 1.0); // in [0,1)
        double h = H0 + (H1 - H0) * unit;
        double perturb = 1.0 + epsilon * rnd(dre);  // in [1-e,1+e)
        double r = perturb * (h * tantheta);
        double symm = rnd(dre);  // in [-1,1)
        double phi = GTE_C_PI * symm;
        double csphi = std::cos(phi);
        double snphi = std::sin(phi);
        X[i] = V + h * U + r * (csphi * W0 + snphi * W1);
    }

    CreatePoints(X);

    Vector3<double> coneVertex;
    Vector3<double> coneAxis;
    double coneAngle;

    CreateGNCone(X, coneVertex, coneAxis, coneAngle);
    Vector4<float> green{ 0.0f, 1.0f, 0.0f, 0.25f };
    mGNCone = CreateConeMesh(X, coneVertex, coneAxis, coneAngle, green);

    CreateLMCone(X, coneVertex, coneAxis, coneAngle);
    Vector4<float> blue{ 0.0f, 0.0f, 1.0f, 0.25f };
    mLMCone = CreateConeMesh(X, coneVertex, coneAxis, coneAngle, blue);
}

void FitConeWindow3::CreatePoints(std::vector<Vector3<double>> const& X)
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    uint32_t numVertices = static_cast<uint32_t>(X.size());
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    auto vertices = vbuffer->Get<Vector3<float>>();
    mCenter = { 0.0f, 0.0f, 0.0f };
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        for (int32_t j = 0; j < 3; ++j)
        {
            vertices[i][j] = static_cast<float>(X[i][j]);
        }
        mCenter += vertices[i];
    }
    mCenter /= static_cast<float>(numVertices);

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYPOINT, numVertices);

    Vector4<float> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, black);

    mPoints = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPoints->localTransform.SetTranslation(-mCenter);

    mPVWMatrices.Subscribe(mPoints->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mPoints);
}

void FitConeWindow3::CreateGNCone(std::vector<Vector3<double>> const& X,
    Vector3<double>& coneVertex, Vector3<double>& coneAxis, double& coneAngle)
{
    ApprCone3<double> fitter;
    size_t const maxIterations = 32;
    double const updateLengthTolerance = 1e-04;
    double const errorDifferenceTolerance = 1e-08;
    bool useConeInputAsInitialGuess = false;

    fitter(static_cast<int32_t>(X.size()), X.data(),
        maxIterations, updateLengthTolerance, errorDifferenceTolerance,
        useConeInputAsInitialGuess, coneVertex, coneAxis, coneAngle);
}

void FitConeWindow3::CreateLMCone(std::vector<Vector3<double>> const& X,
    Vector3<double>& coneVertex, Vector3<double>& coneAxis, double& coneAngle)
{
    ApprCone3<double> fitter;
    size_t const maxIterations = 32;
    double const updateLengthTolerance = 1e-04;
    double const errorDifferenceTolerance = 1e-08;
    double const lambdaFactor = 0.001;
    double const lambdaAdjust = 10.0;
    size_t const maxAdjustments = 8;
    bool useConeInputAsInitialGuess = false;

    fitter(static_cast<int32_t>(X.size()), X.data(),
        maxIterations, updateLengthTolerance, errorDifferenceTolerance,
        lambdaFactor, lambdaAdjust, maxAdjustments, useConeInputAsInitialGuess,
        coneVertex, coneAxis, coneAngle);
}

std::shared_ptr<Visual> FitConeWindow3::CreateConeMesh(std::vector<Vector3<double>> const& X,
    Vector3<double> const& coneVertex, Vector3<double> const& coneAxis, double coneAngle,
    Vector4<float> const& color)
{
    // Compute the cone height extremes.
    double hmin = std::numeric_limits<double>::max();
    double hmax = 0.0;
    for (size_t i = 0; i < X.size(); ++i)
    {
        Vector3<double> diff = X[i] - coneVertex;
        double h = Dot(coneAxis, diff);
        if (h > hmax)
        {
            hmax = h;
        }
        if (h < hmin)
        {
            hmin = h;
        }
    }

    // Compute the tangent of the cone angle.
    double tanTheta = std::tan(coneAngle);

    // Compute a right-handed basis from the cone axis direction;
    Vector3<double> basis[3];
    basis[0] = coneAxis;
    ComputeOrthogonalComplement(1, basis);
    Vector3<double> W0 = basis[1];
    Vector3<double> W1 = basis[2];

    // Create a cone frustum mesh.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    uint32_t const numXSamples = 16;
    uint32_t const numYSamples = 16;
    auto cone = mf.CreateRectangle(numXSamples, numYSamples, 1.0f, 1.0f);
    cone->localTransform.SetTranslation(-mCenter);
    cone->culling = CullingMode::ALWAYS;

    auto vertices = cone->GetVertexBuffer()->Get<Vector3<float>>();
    double const xMult = GTE_C_TWO_PI / static_cast<double>(numYSamples - 1);
    double const YMult = (hmax - hmin) / static_cast<double>(numXSamples - 1);
    for (uint32_t y = 0, i = 0; y < numYSamples; ++y)
    {
        double h = hmin + static_cast<double>(y) * YMult;
        double r = h * tanTheta;
        for (uint32_t x = 0; x < numXSamples; ++x, ++i)
        {
            double phi = static_cast<double>(x) * xMult;
            double rcs = r * std::cos(phi);
            double rsn = r * std::sin(phi);
            Vector3<double> P = coneVertex + h * coneAxis + rcs * W0 + rsn * W1;
            for (int32_t j = 0; j < 3; ++j)
            {
                vertices[i][j] = static_cast<float>(P[j]);
            }
        }
    }

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    cone->SetEffect(effect);

    mPVWMatrices.Subscribe(cone->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(cone);

    return cone;
}
