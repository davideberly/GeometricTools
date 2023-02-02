// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "DistancePointConvexPolyhedronWindow3.h"
#include <Graphics/MeshFactory.h>

DistancePointConvexPolyhedronWindow3::DistancePointConvexPolyhedronWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;
    mEngine->SetBlendState(mBlendState);

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 5000.0f, 0.01f, 0.01f,
        { 0.0f, 0.0f, -6.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    DoQuery();
    mPVWMatrices.Update();
}

void DistancePointConvexPolyhedronWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mPointMesh);
    mEngine->Draw(mPolyhedronMesh);
    mEngine->Draw(mSegment);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool DistancePointConvexPolyhedronWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    float const delta = 0.1f;

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

void DistancePointConvexPolyhedronWindow3::CreateScene()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mPoint = { 1.0f, 1.0f, 1.0f };
    mPointMesh = mf.CreateSphere(8, 8, 0.0625f);
    mPointMesh->localTransform.SetTranslation(mPoint);
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.5f, 0.0f, 0.5f });
    mPointMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mPointMesh->worldTransform, effect->GetPVWMatrixConstant());

    mRedEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.5f, 0.0f, 0.0f, 0.5f });

    mBlueEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.5f, 0.5f });

    mPolyhedronMesh = mf.CreateIcosahedron();
    mPolyhedronMesh->SetEffect(mBlueEffect);
    mPVWMatrices.Subscribe(mPolyhedronMesh->worldTransform, mBlueEffect->GetPVWMatrixConstant());

    std::shared_ptr<VertexBuffer> vbuffer = mPolyhedronMesh->GetVertexBuffer();
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    mPolyhedron.vertices.resize(vbuffer->GetNumElements());
    std::memcpy(mPolyhedron.vertices.data(), vbuffer->GetData(), vbuffer->GetNumBytes());

    std::shared_ptr<IndexBuffer> ibuffer = mPolyhedronMesh->GetIndexBuffer();
    mPolyhedron.indices.resize(ibuffer->GetNumElements());
    std::memcpy(mPolyhedron.indices.data(), ibuffer->GetData(), ibuffer->GetNumBytes());

    mPolyhedronCenter = { 0.0f, 0.0f, 0.0f };
    mPolyhedron.GeneratePlanes();
    mPolyhedron.GenerateAlignedBox();

    vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 0.0f, 1.0f });
    mSegment = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mSegment->worldTransform, effect->GetPVWMatrixConstant());

    mTrackBall.Attach(mPointMesh);
    mTrackBall.Attach(mPolyhedronMesh);
    mTrackBall.Attach(mSegment);
    mTrackBall.Update();
}

void DistancePointConvexPolyhedronWindow3::Translate(int32_t direction, float delta)
{
    for (auto& vertex : mPolyhedron.vertices)
    {
        vertex[direction] += delta;
    }
    mPolyhedronCenter[direction] += delta;
    mPolyhedron.GeneratePlanes();
    mPolyhedron.GenerateAlignedBox();

    auto const& vbuffer = mPolyhedronMesh->GetVertexBuffer();
    std::memcpy(vbuffer->GetData(), mPolyhedron.vertices.data(), vbuffer->GetNumBytes());
    mEngine->Update(vbuffer);

    DoQuery();
    mPVWMatrices.Update();
}

void DistancePointConvexPolyhedronWindow3::Rotate(int32_t direction, float delta)
{
    Matrix3x3<float> rotate = Rotation<3, float>(
        AxisAngle<3, float>(Vector3<float>::Unit(direction), delta));
    for (auto& vertex : mPolyhedron.vertices)
    {
        vertex = mPolyhedronCenter + rotate * (vertex - mPolyhedronCenter);
    }
    mPolyhedron.GeneratePlanes();
    mPolyhedron.GenerateAlignedBox();

    auto const& vbuffer = mPolyhedronMesh->GetVertexBuffer();
    std::memcpy(vbuffer->GetData(), mPolyhedron.vertices.data(), vbuffer->GetNumBytes());
    mEngine->Update(vbuffer);

    DoQuery();
    mPVWMatrices.Update();
}

void DistancePointConvexPolyhedronWindow3::DoQuery()
{
    mPVWMatrices.Unsubscribe(mPolyhedronMesh->worldTransform);

    auto result = mQuery(mPoint, mPolyhedron);
    float const epsilon = 1e-04f;
    if (result.distance > epsilon)
    {
        mPolyhedronMesh->SetEffect(mRedEffect);
        mPVWMatrices.Subscribe(mPolyhedronMesh->worldTransform, mRedEffect->GetPVWMatrixConstant());
    }
    else
    {
        mPolyhedronMesh->SetEffect(mBlueEffect);
        mPVWMatrices.Subscribe(mPolyhedronMesh->worldTransform, mBlueEffect->GetPVWMatrixConstant());
    }

    auto* vertices = mSegment->GetVertexBuffer()->Get<Vector3<float>>();
    vertices[0] = result.closest[0];
    vertices[1] = result.closest[1];
    mEngine->Update(mSegment->GetVertexBuffer());
}
