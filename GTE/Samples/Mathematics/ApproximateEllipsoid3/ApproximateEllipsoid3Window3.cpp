// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include "ApproximateEllipsoid3Window3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>
#include <random>

ApproximateEllipsoid3Window3::ApproximateEllipsoid3Window3(Parameters& parameters)
    :
    Window3(parameters),
    mFitter{},
    mTrueEllipsoid{},
    mApprEllipsoid{},
    mTrueMesh{},
    mApprMesh{}
{
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::WIREFRAME;

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 1000.0f, 0.001f, 0.001f,
        { -8.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void ApproximateEllipsoid3Window3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    std::array<float, 4> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    mEngine->Draw(8, 24, black, "The blue mesh is the true ellipsoid.");
    mEngine->Draw(8, 48, black, "The red mesh is the fitted ellipsoid for perturbed points from the true ellipsoid.");
    mEngine->Draw(8, mYSize - 8, black, mTimer.GetFPS());
    mEngine->Draw(mTrueMesh);
    mEngine->Draw(mApprMesh);
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool ApproximateEllipsoid3Window3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mWireState)
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

void ApproximateEllipsoid3Window3::CreateScene()
{
    mTrueEllipsoid.center = { 0.0f, 0.0f, 0.0f };
    mTrueEllipsoid.axis[0] = { 1.0f, 2.0f, 3.0f };
    Normalize(mTrueEllipsoid.axis[0]);
    ComputeOrthogonalComplement(1, mTrueEllipsoid.axis.data());
    mTrueEllipsoid.extent = { 1.0f, 2.0f, 3.0f };

    mTrueMesh = CreateEllipsoidMesh(mTrueEllipsoid, Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f });
    Matrix3x3<float> rotate{};
    rotate.SetCol(0, mTrueEllipsoid.axis[0]);
    rotate.SetCol(1, mTrueEllipsoid.axis[1]);
    rotate.SetCol(2, mTrueEllipsoid.axis[2]);

    std::default_random_engine dre{};
    std::uniform_real_distribution<float> urd(-1.0f, 1.0f);
    float amplitude = 0.01f;
    size_t numPoints = 1024;
    std::vector<Vector3<float>> points(numPoints);
    for (auto& p : points)
    {
        p = { urd(dre), urd(dre), urd(dre) };
        Normalize(p);
        p[0] *= mTrueEllipsoid.extent[0];
        p[1] *= mTrueEllipsoid.extent[1];
        p[2] *= mTrueEllipsoid.extent[2];
        p = rotate * p + mTrueEllipsoid.center +
            amplitude * Vector3<float>{ urd(dre), urd(dre), urd(dre) };
    }

    mFitter(points, 1024, false, mApprEllipsoid);
    mApprMesh = CreateEllipsoidMesh(mApprEllipsoid, Vector4<float>{ 1.0f, 0.0f, 0.0f, 1.0f });
}

std::shared_ptr<Visual> ApproximateEllipsoid3Window3::CreateEllipsoidMesh(
    Ellipsoid3<float> const& ellipsoid, Vector4<float> const& color)
{
    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf(vformat);

    auto mesh = mf.CreateSphere(64, 64, 1.0f);
    auto const& vbuffer = mesh->GetVertexBuffer();
    auto* vertices = vbuffer->Get<Vector3<float>>();
    uint32_t numVertices = vbuffer->GetNumElements();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i][0] *= ellipsoid.extent[0];
        vertices[i][1] *= ellipsoid.extent[1];
        vertices[i][2] *= ellipsoid.extent[2];
    }

    Matrix3x3<float> rotate{};
    rotate.SetCol(0, ellipsoid.axis[0]);
    rotate.SetCol(1, ellipsoid.axis[1]);
    rotate.SetCol(2, ellipsoid.axis[2]);
    mesh->localTransform.SetRotation(rotate);
    mesh->localTransform.SetTranslation(ellipsoid.center);

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mesh->SetEffect(effect);

    mPVWMatrices.Subscribe(mesh);
    mTrackBall.Attach(mesh);
    return mesh;
}

