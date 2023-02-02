// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "DistanceAlignedBoxOrientedBoxWindow3.h"
#include <Graphics/MeshFactory.h>

DistanceAlignedBoxOrientedBoxWindow3::DistanceAlignedBoxOrientedBoxWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mEngine->SetRasterizerState(mNoCullState);

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;
    mEngine->SetBlendState(mBlendState);

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 5000.0f, 0.1f, 0.01f,
        { 0.0f, 0.0f, -24.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    DoQuery();
    mPVWMatrices.Update();
}

void DistanceAlignedBoxOrientedBoxWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mBox0Mesh);
    mEngine->Draw(mBox1Mesh);
    mEngine->Draw(mSegment);
    mEngine->Draw(mPoint0);
    mEngine->Draw(mPoint1);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool DistanceAlignedBoxOrientedBoxWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    float const delta = 0.1f;

    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mNoCullState)
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        return true;

    case ' ':
        DoQuery();
        return true;

    case 'x':  // decrement x-center of box
        Translate(0, -delta);
        return true;

    case 'X':  // increment x-center of box
        Translate(0, +delta);
        return true;

    case 'y':  // decrement y-center of box
        Translate(1, -delta);
        return true;

    case 'Y':  // increment y-center of box
        Translate(1, +delta);
        return true;

    case 'z':  // decrement z-center of box
        Translate(2, -delta);
        return true;

    case 'Z':  // increment z-center of box
        Translate(2, +delta);
        return true;

    case 'p':  // rotate about axis[0]
        Rotate(0, -delta);
        return true;

    case 'P':  // rotate about axis[0]
        Rotate(0, +delta);
        return true;

    case 'r':  // rotate about axis[1]
        Rotate(1, -delta);
        return true;

    case 'R':  // rotate about axis[1]
        Rotate(1, +delta);
        return true;

    case 'h':  // rotate about axis[2]
        Rotate(2, -delta);
        return true;

    case 'H':  // rotate about axis[2]
        Rotate(2, +delta);
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void DistanceAlignedBoxOrientedBoxWindow3::CreateScene()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mBox0.min = { -1.0f, -1.0f, -1.0f };
    mBox0.max = { 1.0f, 1.0f, 1.0f };

    mBox1.center = { 2.5f, 3.0f, 3.5f };
    mBox1.axis[0] = { 1.0f, 0.0f, 0.0f };
    mBox1.axis[1] = { 0.0f, 1.0f, 0.0f };
    mBox1.axis[2] = { 0.0f, 0.0f, 1.0f };
    mBox1.extent = { 0.5f, 1.0f, 1.5f };

    Vector3<float> extent0 = 0.5f * (mBox0.max - mBox0.min);
    mBox0Mesh = mf.CreateBox(extent0[0], extent0[1], extent0[2]);

    mBox1Mesh = mf.CreateBox(mBox1.extent[0], mBox1.extent[1], mBox1.extent[2]);
    mBox1Mesh->localTransform.SetTranslation(mBox1.center);

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.5f, 0.0f, 0.5f });
    mBox0Mesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mBox0Mesh->worldTransform, effect->GetPVWMatrixConstant());

    mRedEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.5f, 0.0f, 0.0f, 0.5f });

    mBlueEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.5f, 0.5f });

    mBox1Mesh->SetEffect(mBlueEffect);
    mPVWMatrices.Subscribe(mBox1Mesh->worldTransform, mBlueEffect->GetPVWMatrixConstant());

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.0f, 1.0f });
    mSegment = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mSegment->worldTransform, effect->GetPVWMatrixConstant());

    mPoint0 = mf.CreateSphere(8, 8, 0.0625f);
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.0f, 1.0f });
    mPoint0->SetEffect(effect);
    mPVWMatrices.Subscribe(mPoint0->worldTransform, effect->GetPVWMatrixConstant());

    mPoint1 = mf.CreateSphere(8, 8, 0.0625f);
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.0f, 1.0f });
    mPoint1->SetEffect(effect);
    mPVWMatrices.Subscribe(mPoint1->worldTransform, effect->GetPVWMatrixConstant());

    mTrackBall.Attach(mBox0Mesh);
    mTrackBall.Attach(mBox1Mesh);
    mTrackBall.Attach(mSegment);
    mTrackBall.Attach(mPoint0);
    mTrackBall.Attach(mPoint1);
    mTrackBall.Update();
}

void DistanceAlignedBoxOrientedBoxWindow3::Translate(int32_t direction, float delta)
{
    mBox1.center[direction] += delta;
    mBox1Mesh->localTransform.SetTranslation(mBox1.center);
    mBox1Mesh->Update();
    DoQuery();
    mPVWMatrices.Update();
}

void DistanceAlignedBoxOrientedBoxWindow3::Rotate(int32_t direction, float delta)
{
    Quaternion<float> incr = Rotation<3, float>(
        AxisAngle<3, float>(mBox1.axis[direction], delta));
    for (int32_t i = 0; i < 3; ++i)
    {
        if (i != direction)
        {
            mBox1.axis[i] = HProject(gte::Rotate(incr, HLift(mBox1.axis[i], 0.0f)));
        }
    }
    Quaternion<float> q;
    mBox1Mesh->localTransform.GetRotation(q);
    mBox1Mesh->localTransform.SetRotation(incr * q);
    mBox1Mesh->Update();
    DoQuery();
    mPVWMatrices.Update();
}

void DistanceAlignedBoxOrientedBoxWindow3::DoQuery()
{
    mPVWMatrices.Unsubscribe(mBox1Mesh->worldTransform);

    auto result = mQuery(mBox0, mBox1);
    float const epsilon = 1e-04f;
    if (result.distance > epsilon)
    {
        mBox1Mesh->SetEffect(mRedEffect);
        mPVWMatrices.Subscribe(mBox1Mesh->worldTransform, mRedEffect->GetPVWMatrixConstant());
    }
    else
    {
        mBox1Mesh->SetEffect(mBlueEffect);
        mPVWMatrices.Subscribe(mBox1Mesh->worldTransform, mBlueEffect->GetPVWMatrixConstant());
    }

    auto* vertices = mSegment->GetVertexBuffer()->Get<Vector3<float>>();
    vertices[0] = result.closest[0];
    vertices[1] = result.closest[1];
    mEngine->Update(mSegment->GetVertexBuffer());

    mPoint0->localTransform.SetTranslation(vertices[0]);
    mPoint1->localTransform.SetTranslation(vertices[1]);
    mTrackBall.Update();
}
