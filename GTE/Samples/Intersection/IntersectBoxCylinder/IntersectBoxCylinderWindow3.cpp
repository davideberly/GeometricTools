// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "IntersectBoxCylinderWindow3.h"
#include <Graphics/MeshFactory.h>

IntersectBoxCylinderWindow3::IntersectBoxCylinderWindow3(Parameters& parameters)
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

    TestIntersection();
    mPVWMatrices.Update();
}

void IntersectBoxCylinderWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mCylinderMesh);
    mEngine->Draw(mBoxMesh);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectBoxCylinderWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
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
        TestIntersection();
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

void IntersectBoxCylinderWindow3::CreateScene()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mCylinder.axis.origin = { 3.0f, 2.0f, 3.0f };
    mCylinder.axis.direction = { 0.0f, 0.0f, 1.0f };
    mCylinder.radius = 1.0f;
    mCylinder.height = 8.0f;

    mCylinderMesh = mf.CreateCylinderClosed(16, 16, mCylinder.radius, mCylinder.height);
    mCylinderMesh->localTransform.SetTranslation(mCylinder.axis.origin);
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
            Vector4<float>{ 0.0f, 0.5f, 0.0f, 0.5f });
    mCylinderMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mCylinderMesh->worldTransform, effect->GetPVWMatrixConstant());

    mRedEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.5f, 0.0f, 0.0f, 0.5f });

    mBlueEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.5f, 0.5f });

    mBox.center = { 0.0f, 0.0f, 0.0f };
    mBox.axis[0] = { 1.0f, 0.0f, 0.0f };
    mBox.axis[1] = { 0.0f, 1.0f, 0.0f };
    mBox.axis[2] = { 0.0f, 0.0f, 1.0f };
    mBox.extent = { 1.0f, 2.0f, 3.0f };

    mBoxMesh = mf.CreateBox(mBox.extent[0], mBox.extent[1], mBox.extent[2]);
    mBoxMesh->SetEffect(mBlueEffect);
    mPVWMatrices.Subscribe(mBoxMesh->worldTransform, mBlueEffect->GetPVWMatrixConstant());

    mTrackBall.Attach(mCylinderMesh);
    mTrackBall.Attach(mBoxMesh);
    mTrackBall.Update();
}

void IntersectBoxCylinderWindow3::Translate(int32_t direction, float delta)
{
    mBox.center[direction] += delta;
    mBoxMesh->localTransform.SetTranslation(mBox.center);
    mBoxMesh->Update();
    TestIntersection();
    mPVWMatrices.Update();
}

void IntersectBoxCylinderWindow3::Rotate(int32_t direction, float delta)
{
    Quaternion<float> incr = Rotation<3, float>(
        AxisAngle<3, float>(mBox.axis[direction], delta));
    for (int32_t i = 0; i < 3; ++i)
    {
        if (i != direction)
        {
            mBox.axis[i] = HProject(
                gte::Rotate(incr, HLift(mBox.axis[i], 0.0f)));
        }
    }
    Quaternion<float> q;
    mBoxMesh->localTransform.GetRotation(q);
    mBoxMesh->localTransform.SetRotation(incr * q);
    mBoxMesh->Update();
    TestIntersection();
    mPVWMatrices.Update();
}

void IntersectBoxCylinderWindow3::TestIntersection()
{
    mPVWMatrices.Unsubscribe(mBoxMesh->worldTransform);

    if (mQuery(mBox, mCylinder).intersect)
    {
        mBoxMesh->SetEffect(mRedEffect);
        mPVWMatrices.Subscribe(mBoxMesh->worldTransform, mRedEffect->GetPVWMatrixConstant());
    }
    else
    {
        mBoxMesh->SetEffect(mBlueEffect);
        mPVWMatrices.Subscribe(mBoxMesh->worldTransform, mBlueEffect->GetPVWMatrixConstant());
    }
}
