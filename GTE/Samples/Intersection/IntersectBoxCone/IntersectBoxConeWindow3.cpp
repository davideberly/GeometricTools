// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "IntersectBoxConeWindow3.h"
#include <Graphics/MeshFactory.h>

IntersectBoxConeWindow3::IntersectBoxConeWindow3(Parameters& parameters)
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

void IntersectBoxConeWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    if (mCone.GetMinHeight() == 0.0f)
    {
        mEngine->Draw(mConeH0Mesh);
    }
    else
    {
        mEngine->Draw(mConeH4Mesh);
    }

    mEngine->Draw(mDiskMaxMesh);
    mEngine->Draw(mBoxMesh);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectBoxConeWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
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

    case 'm':  // toggle between a minimum height of 0 and 4
    case 'M':
        mCone.MakeConeFrustum(4.0f - mCone.GetMinHeight(), mCone.GetMaxHeight());
        TestIntersection();
        mPVWMatrices.Update();
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

void IntersectBoxConeWindow3::CreateScene()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mCone.ray.origin = { 0.0f, 0.0f, 0.0f }; // { 1.0f, 1.0f, 1.0f };
    mCone.ray.direction = { 0.0f, 0.0f, 1.0f };
    mCone.SetAngle(0.25f);
    mCone.MakeConeFrustum(0.0f, 16.0f);

    // Create a visual representation of the cone with heights in [0,16].
    float const tanAngle = std::tan(mCone.angle);
    float const maxRadius = mCone.GetMaxHeight() * tanAngle;
    mConeH0Mesh = mf.CreateDisk(16, 16, maxRadius);
    mConeH0Mesh->localTransform.SetTranslation(mCone.ray.origin);
    std::shared_ptr<VertexBuffer> vbuffer = mConeH0Mesh->GetVertexBuffer();
    uint32_t numVertices = vbuffer->GetNumElements();
    auto vertices = vbuffer->Get<Vector3<float>>();
    float cotAngle = mCone.cosAngle / mCone.sinAngle;
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        Vector3<float>& P = vertices[i];
        P[2] = cotAngle * std::sqrt(P[0] * P[0] + P[1] * P[1]);
    }

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.5f, 0.0f, 0.5f });
    mConeH0Mesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mConeH0Mesh->worldTransform, effect->GetPVWMatrixConstant());

    // Create a visual representation of the cone with heights in [4,16].
    uint32_t const numAxial = 16;
    uint32_t const numRadial = 16;
    mConeH4Mesh = mf.CreateCylinderOpen(numAxial, numRadial, 1.0f, 1.0f);
    mConeH4Mesh->localTransform.SetTranslation(mCone.ray.origin);
    vbuffer = mConeH4Mesh->GetVertexBuffer();
    vertices = vbuffer->Get<Vector3<float>>();
    for (uint32_t row = 0, i = 0; row < numAxial; ++row)
    {
        float const height = 4.0f + 12.0f * static_cast<float>(row) / static_cast<float>(numAxial - 1);
        float const radius = height * tanAngle;
        for (uint32_t col = 0; col <= numRadial; ++col, ++i)
        {
            Vector3<float>& P = vertices[i];
            float stretch = radius / std::sqrt(P[0] * P[0] + P[1] * P[1]);
            P[0] *= stretch;
            P[1] *= stretch;
            P[2] = height;
        }
    }

    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.5f, 0.0f, 0.5f });
    mConeH4Mesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mConeH4Mesh->worldTransform, effect->GetPVWMatrixConstant());

    // Create a visual representation of the maximum height disk cap for
    // either cone.
    mDiskMaxMesh = mf.CreateDisk(16, 16, maxRadius);
    mDiskMaxMesh->localTransform.SetTranslation(mCone.ray.origin + mCone.GetMaxHeight() * mCone.ray.direction);
    mDiskMaxMesh->Update();
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.5f, 0.0f, 0.5f });
    mDiskMaxMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mDiskMaxMesh->worldTransform, effect->GetPVWMatrixConstant());

    mRedEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.5f, 0.0f, 0.0f, 0.5f });

    mBlueEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.5f, 0.5f });

    Vector3<float> extent{ 1.0f, 2.0f, 3.0f };
#if defined(USE_ORIENTED_BOX)
    mBox.center = { 0.0f, 0.0f, 0.0f };
    mBox.axis[0] = { 1.0f, 0.0f, 0.0f };
    mBox.axis[1] = { 0.0f, 1.0f, 0.0f };
    mBox.axis[2] = { 0.0f, 0.0f, 1.0f };
    mBox.extent = extent;
#else
    mBox.min = -extent;
    mBox.max = +extent;
#endif

    mBoxMesh = mf.CreateBox(extent[0], extent[1], extent[2]);
    mBoxMesh->SetEffect(mBlueEffect);
    mPVWMatrices.Subscribe(mBoxMesh->worldTransform, mBlueEffect->GetPVWMatrixConstant());

    mTrackBall.Attach(mConeH0Mesh);
    mTrackBall.Attach(mConeH4Mesh);
    mTrackBall.Attach(mDiskMaxMesh);
    mTrackBall.Attach(mBoxMesh);
    mTrackBall.Update();
}

void IntersectBoxConeWindow3::Translate(int32_t direction, float delta)
{
#if defined(USE_ORIENTED_BOX)
    mBox.center[direction] += delta;
    mBoxMesh->localTransform.SetTranslation(mBox.center);
#else
    mBox.min[direction] += delta;
    mBox.max[direction] += delta;
    mBoxMesh->localTransform.SetTranslation(0.5f*(mBox.min + mBox.max));
#endif
    mBoxMesh->Update();
    TestIntersection();
    mPVWMatrices.Update();
}

void IntersectBoxConeWindow3::Rotate(int32_t direction, float delta)
{
#if defined(USE_ORIENTED_BOX)
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
#else
    (void)direction;
    (void)delta;
#endif
}

void IntersectBoxConeWindow3::TestIntersection()
{
    mPVWMatrices.Unsubscribe(mBoxMesh->worldTransform);

    if (mQuery(mBox, mCone).intersect)
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
