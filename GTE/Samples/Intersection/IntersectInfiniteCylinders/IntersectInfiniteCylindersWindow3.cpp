// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "IntersectInfiniteCylindersWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>

IntersectInfiniteCylindersWindow3::IntersectInfiniteCylindersWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mC0(4.0f),
    mW1(3.0f / 5.0f),
    mW2(4.0f / 5.0f),
    mRadius0(3.0f),
    mRadius1(2.0f),
    mHeight(100.0f),
    mAngle(std::atan2(mW1, mW2))
{
    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f});
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.01f, 0.001f,
        { 0.0f, -16.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();
}

void IntersectInfiniteCylindersWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    if (mCylinder0->culling == CullingMode::NEVER)
    {
        mEngine->Draw(mCylinder0);
    }

    if (mCylinder1->culling == CullingMode::NEVER)
    {
        mEngine->Draw(mCylinder1);
    }

    mEngine->Draw(mCurve0);
    mEngine->Draw(mCurve1);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectInfiniteCylindersWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
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

    case '0':
        if (mCylinder0->culling == CullingMode::NEVER)
        {
            mCylinder0->culling = CullingMode::ALWAYS;
        }
        else
        {
            mCylinder0->culling = CullingMode::NEVER;
        }
        return true;

    case '1':
        if (mCylinder1->culling == CullingMode::NEVER)
        {
            mCylinder1->culling = CullingMode::ALWAYS;
        }
        else
        {
            mCylinder1->culling = CullingMode::NEVER;
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void IntersectInfiniteCylindersWindow3::CreateScene()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    // Create the canonical cylinder.
    mCylinder0 = mf.CreateCylinderOpen(32, 128, mRadius0, mHeight);
    mCylinder0->culling = CullingMode::NEVER;
    Vector4<float> red{ 0.5f, 0.0f, 0.0f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, red);
    mCylinder0->SetEffect(effect);
    mPVWMatrices.Subscribe(mCylinder0->worldTransform, effect->GetPVWMatrixConstant());

    // Create the other cylinder.
    mCylinder1 = mf.CreateCylinderOpen(32, 128, mRadius1, mHeight);
    mCylinder1->culling = CullingMode::NEVER;
    mCylinder1->localTransform.SetRotation(
        AxisAngle<4, float>(Vector4<float>::Unit(0), -mAngle));
    mCylinder1->localTransform.SetTranslation(mC0, 0.0f, 0.0f);
    Vector4<float> blue{ 0.0f, 0.0f, 0.5f, 1.0f };
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, blue);
    mCylinder1->SetEffect(effect);
    mPVWMatrices.Subscribe(mCylinder1->worldTransform, effect->GetPVWMatrixConstant());

    // Create the intersection curve.
    uint32_t numVertices = 1024;
    float const minTheta = static_cast<float>(2.0 * GTE_C_PI / 3.0f);
    float const maxTheta = static_cast<float>(4.0 * GTE_C_PI / 3.0f);
    float multiplier = (maxTheta - minTheta) / static_cast<float>(numVertices - 1);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_CONTIGUOUS, numVertices - 1);
    Vector4<float> green{ 0.0f, 0.5f, 0.0f, 1.0f };

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    auto vertices = vbuffer->Get<Vector3<float>>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        float theta = minTheta + multiplier * i;
        float cs = std::cos(theta);
        float sn = std::sin(theta);
        float tmp = mC0 + mRadius1 * cs;
        float discr = std::fabs(mRadius0 * mRadius0 - tmp * tmp);
        float t = (-mRadius1 * mW2 * sn - std::sqrt(discr)) / mW1;
        vertices[i][0] = mC0 + mRadius1 * cs;
        vertices[i][1] = +mRadius1 * sn * mW2 + t * mW1;
        vertices[i][2] = -mRadius1 * sn * mW1 + t * mW2;
    }
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, green);
    mCurve0 = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mCurve0->worldTransform, effect->GetPVWMatrixConstant());

    vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vertices = vbuffer->Get<Vector3<float>>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        float theta = minTheta + multiplier * i;
        float cs = std::cos(theta);
        float sn = std::sin(theta);
        float tmp = mC0 + mRadius1 * cs;
        float discr = std::fabs(mRadius0 * mRadius0 - tmp * tmp);
        float t = (-mRadius1 * mW2 * sn + std::sqrt(discr)) / mW1;
        vertices[i][0] = mC0 + mRadius1 * cs;
        vertices[i][1] = +mRadius1 * sn * mW2 + t * mW1;
        vertices[i][2] = -mRadius1 * sn * mW1 + t * mW2;
    }
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, green);
    mCurve1 = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mCurve1->worldTransform, effect->GetPVWMatrixConstant());

    mTrackBall.Attach(mCylinder0);
    mTrackBall.Attach(mCylinder1);
    mTrackBall.Attach(mCurve0);
    mTrackBall.Attach(mCurve1);
    mTrackBall.Update();
    mPVWMatrices.Update();
}
