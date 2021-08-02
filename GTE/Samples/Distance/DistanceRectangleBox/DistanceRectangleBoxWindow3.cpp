// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.08.01

#include "DistanceRectangleBoxWindow3.h"
#include <Graphics/MeshFactory.h>

DistanceRectangleBoxWindow3::DistanceRectangleBoxWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cullMode = RasterizerState::CULL_NONE;
    mEngine->SetRasterizerState(mNoCullState);

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;
    mEngine->SetBlendState(mBlendState);

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 5000.0f, 0.1f, 0.01f,
        { 0.0f, 0.0f, -24.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    DoQuery();
    mPVWMatrices.Update();
}

void DistanceRectangleBoxWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mRectangleMesh);
    mEngine->Draw(mBoxMesh);
    mEngine->Draw(mSegment);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool DistanceRectangleBoxWindow3::OnCharPress(unsigned char key, int x, int y)
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

void DistanceRectangleBoxWindow3::CreateScene()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mRectangle.center = { 4.5f, 2.0f, 3.0f };
    mRectangle.axis[0] = { 1.0f, 0.0f, 0.0f };
    mRectangle.axis[1] = { 0.0f, 1.0f, 1.0f };
    Normalize(mRectangle.axis[1]);
    mRectangle.extent = { 2.0f, 1.0f };

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    auto& vertices = *vbuffer->Get<std::array<Vector3<float>, 4>>();
    mRectangle.GetVertices(vertices);

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(unsigned int));
    auto* indices = ibuffer->Get<unsigned int>();
    indices[0] = 0;  indices[1] = 1;  indices[2] = 2;
    indices[3] = 1;  indices[4] = 3;  indices[5] = 2;

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.5f, 0.0f, 0.5f });
    mRectangleMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mRectangleMesh->worldTransform, effect->GetPVWMatrixConstant());

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

    vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.0f, 1.0f });
    mSegment = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mSegment->worldTransform, effect->GetPVWMatrixConstant());

    mTrackBall.Attach(mRectangleMesh);
    mTrackBall.Attach(mBoxMesh);
    mTrackBall.Attach(mSegment);
    mTrackBall.Update();
}

void DistanceRectangleBoxWindow3::Translate(int direction, float delta)
{
    mBox.center[direction] += delta;
    mBoxMesh->localTransform.SetTranslation(mBox.center);
    mBoxMesh->Update();
    DoQuery();
    mPVWMatrices.Update();
}

void DistanceRectangleBoxWindow3::Rotate(int direction, float delta)
{
    Quaternion<float> incr = Rotation<3, float>(
        AxisAngle<3, float>(mBox.axis[direction], delta));
    for (int i = 0; i < 3; ++i)
    {
        if (i != direction)
        {
            mBox.axis[i] = HProject(gte::Rotate(incr, HLift(mBox.axis[i], 0.0f)));
        }
    }
    Quaternion<float> q;
    mBoxMesh->localTransform.GetRotation(q);
    mBoxMesh->localTransform.SetRotation(incr * q);
    mBoxMesh->Update();
    DoQuery();
    mPVWMatrices.Update();
}

void DistanceRectangleBoxWindow3::DoQuery()
{
    mPVWMatrices.Unsubscribe(mBoxMesh->worldTransform);

    auto result = mQuery(mRectangle, mBox);
    float const epsilon = 1e-04f;
    if (result.distance > epsilon)
    {
        mBoxMesh->SetEffect(mRedEffect);
        mPVWMatrices.Subscribe(mBoxMesh->worldTransform, mRedEffect->GetPVWMatrixConstant());
    }
    else
    {
        mBoxMesh->SetEffect(mBlueEffect);
        mPVWMatrices.Subscribe(mBoxMesh->worldTransform, mBlueEffect->GetPVWMatrixConstant());
    }

    auto* vertices = mSegment->GetVertexBuffer()->Get<Vector3<float>>();
    vertices[0] = result.closest[0];
    vertices[1] = result.closest[1];
    mEngine->Update(mSegment->GetVertexBuffer());
}
