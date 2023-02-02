// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.20

#include "CollisionsBoundTreeWindow3.h"
#include <Mathematics/Rotation.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>

CollisionsBoundTreeWindow3::CollisionsBoundTreeWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mNoCullState{},
    mNoCullWireState{},
    mCylinder0{},
    mCylinder1{},
    mCylinderMesh0{},
    mCylinderMesh1{},
    mGroup{},
    mBlueUV{ 0.25f, 0.25f },
    mRedUV{ 0.25f, 0.75f },
    mCyanUV{ 0.75f, 0.25f },
    mYellowUV{ 0.75f, 0.75f }
{
    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetRasterizerState(mNoCullState);

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.001f, 0.001f,
        { 4.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f});

    CreateScene();
}

void CollisionsBoundTreeWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mCylinder0);
    mEngine->Draw(mCylinder1);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool CollisionsBoundTreeWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    if (Transform(key))
    {
        return true;
    }

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
    }

    return Window3::OnCharPress(key, x, y);
}

void CollisionsBoundTreeWindow3::CreateScene()
{
    // Create a texture image to be used by both cylinders.
    auto texture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, 2, 2);
    auto* texels = texture->Get<uint32_t>();
    texels[0] = 0xFFFF0000;  // blue
    texels[1] = 0xFFFFFF00;  // cyan
    texels[2] = 0xFF0000FF;  // red
    texels[3] = 0xFF00FFFF;  // yellow

    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf{};
    mf.SetVertexFormat(vformat);

    mCylinder0 = mf.CreateCylinderClosed(8, 16, 1.0f, 2.0f);
    auto const& vbuffer0 = mCylinder0->GetVertexBuffer();
    vbuffer0->SetUsage(Resource::DYNAMIC_UPDATE);
    auto* vertices0 = vbuffer0->Get<Vertex>();
    uint32_t const numVertices0 = vbuffer0->GetNumElements();
    for (uint32_t i = 0; i < numVertices0; ++i)
    {
        vertices0[i].tcoord = mBlueUV;
    }
    auto effect0 = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::CLAMP,
        SamplerState::Mode::CLAMP);
    mCylinder0->SetEffect(effect0);
    mPVWMatrices.Subscribe(mCylinder0);
    mTrackBall.Attach(mCylinder0);

    mCylinder1 = mf.CreateCylinderClosed(16, 8, 0.25f, 4.0f);
    auto const& vbuffer1 = mCylinder1->GetVertexBuffer();
    auto* vertices1 = vbuffer1->Get<Vertex>();
    vbuffer1->SetUsage(Resource::DYNAMIC_UPDATE);
    uint32_t const numVertices1 = vbuffer1->GetNumElements();
    for (uint32_t i = 0; i < numVertices1; ++i)
    {
        vertices1[i].tcoord = mRedUV;
    }
    auto effect1 = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::CLAMP,
        SamplerState::Mode::CLAMP);
    mCylinder1->SetEffect(effect1);
    mPVWMatrices.Subscribe(mCylinder1);
    mTrackBall.Attach(mCylinder1);

    mTrackBall.Update();
    mPVWMatrices.Update();

    // Set up the collision system. Record0 handles the collision response.
    // Record1 is not given a callback so that 'double processing' of the
    // events does not occur.
    Vector3<float> velocity{ 0.0f, 0.0f, 0.0f };

    auto ResponseFunction =
        [this](CRecord& record0, int32_t t0, CRecord& record1, int32_t t1,
            float contactTime)
    {
        Response(record0, t0, record1, t1, contactTime);
    };

    mCylinderMesh0 = std::make_shared<Mesh>(mCylinder0);
    auto tree0 = std::make_shared<CTree>(mCylinderMesh0, 1, false);
    auto tiCallback0 = std::make_shared<CRecord::TICallback>(ResponseFunction);
    std::shared_ptr<CRecord::FICallback> fiCallback0{};
    auto record0 = std::make_shared<CRecord>(tree0, velocity, tiCallback0, fiCallback0);

    mCylinderMesh1 = std::make_shared<Mesh>(mCylinder1);
    auto tree1 = std::make_shared<CTree>(mCylinderMesh1, 1, false);
    std::shared_ptr<CRecord::TICallback> tiCallback1{};
    std::shared_ptr<CRecord::FICallback> fiCallback1{};
    auto record1 = std::make_shared<CRecord>(tree1, velocity, tiCallback1, fiCallback1);

    mGroup = std::make_shared<CGroup>();
    mGroup->Insert(record0);
    mGroup->Insert(record1);

    ResetColors();
    mGroup->TestIntersection();
}

bool CollisionsBoundTreeWindow3::Transform(uint8_t key)
{
    // Move the tall/thin cylinder. After each motion, reset the texture
    // coordinates to the "no intersection" state, then allow the collision
    // system test for intersection. Any intersecting triangles have their
    // texture coordinates changed to the "intersection" state.

    float constexpr trnSpeed = 0.1f;
    float constexpr rotSpeed = 0.1f;

    Matrix4x4<float> rot{}, incr{};
    AxisAngle<4, float> aa{};
    Vector3<float> trn{};

    switch (key)
    {
    case 'x':
        trn = mCylinder1->localTransform.GetTranslation();
        trn[0] -= trnSpeed;
        mCylinder1->localTransform.SetTranslation(trn);
        break;
    case 'X':
        trn = mCylinder1->localTransform.GetTranslation();
        trn[0] += trnSpeed;
        mCylinder1->localTransform.SetTranslation(trn);
        break;
    case 'y':
        trn = mCylinder1->localTransform.GetTranslation();
        trn[1] -= trnSpeed;
        mCylinder1->localTransform.SetTranslation(trn);
        break;
    case 'Y':
        trn = mCylinder1->localTransform.GetTranslation();
        trn[1] += trnSpeed;
        mCylinder1->localTransform.SetTranslation(trn);
        break;
    case 'z':
        trn = mCylinder1->localTransform.GetTranslation();
        trn[2] -= trnSpeed;
        mCylinder1->localTransform.SetTranslation(trn);
        break;
    case 'Z':
        trn = mCylinder1->localTransform.GetTranslation();
        trn[2] += trnSpeed;
        mCylinder1->localTransform.SetTranslation(trn);
        break;
    case 'r':
        rot = mCylinder1->localTransform.GetRotation();
        aa.axis = { 1.0f, 0.0f, 0.0f, 0.0f };
        aa.angle = rotSpeed;
        incr = Rotation<4, float>(aa);
        mCylinder1->localTransform.SetRotation(incr * rot);
        break;
    case 'R':
        rot = mCylinder1->localTransform.GetRotation();
        aa.axis = { 1.0f, 0.0f, 0.0f, 0.0f };
        aa.angle = -rotSpeed;
        incr = Rotation<4, float>(aa);
        mCylinder1->localTransform.SetRotation(incr * rot);
        break;
    case 'a':
        rot = mCylinder1->localTransform.GetRotation();
        aa.axis = { 0.0f, 1.0f, 0.0f, 0.0f };
        aa.angle = rotSpeed;
        incr = Rotation<4, float>(aa);
        mCylinder1->localTransform.SetRotation(incr * rot);
        break;
    case 'A':
        rot = mCylinder1->localTransform.GetRotation();
        aa.axis = { 0.0f, 1.0f, 0.0f, 0.0f };
        aa.angle = -rotSpeed;
        incr = Rotation<4, float>(aa);
        mCylinder1->localTransform.SetRotation(incr * rot);
        break;
    case 'p':
        rot = mCylinder1->localTransform.GetRotation();
        aa.axis = { 0.0f, 0.0f, 1.0f, 0.0f };
        aa.angle = rotSpeed;
        incr = Rotation<4, float>(aa);
        mCylinder1->localTransform.SetRotation(incr * rot);
        break;
    case 'P':
        rot = mCylinder1->localTransform.GetRotation();
        aa.axis = { 0.0f, 0.0f, 1.0f, 0.0f };
        aa.angle = -rotSpeed;
        incr = Rotation<4, float>(aa);
        mCylinder1->localTransform.SetRotation(incr * rot);
        break;
    default:
        return false;
    }

    // Activate the collision system.
    mCylinder1->Update();
    ResetColors();
    mGroup->TestIntersection();
    mTrackBall.Update();
    mPVWMatrices.Update();
    return true;
}

void CollisionsBoundTreeWindow3::ResetColors()
{
    auto const& vbuffer0 = mCylinder0->GetVertexBuffer();
    auto* vertices0 = vbuffer0->Get<Vertex>();
    uint32_t const numVertices0 = vbuffer0->GetNumElements();
    for (uint32_t i = 0; i < numVertices0; ++i)
    {
        vertices0[i].tcoord = mBlueUV;
    }
    mEngine->Update(mCylinder0->GetVertexBuffer());

    auto const& vbuffer1 = mCylinder1->GetVertexBuffer();
    auto* vertices1 = vbuffer1->Get<Vertex>();
    uint32_t const numVertices1 = vbuffer1->GetNumElements();
    for (uint32_t i = 0; i < numVertices1; ++i)
    {
        vertices1[i].tcoord = mRedUV;
    }
    mEngine->Update(mCylinder1->GetVertexBuffer());
}

void CollisionsBoundTreeWindow3::Response(CRecord& record0, int32_t t0,
    CRecord& record1, int32_t t1, float contactTime)
{
    // The contact time is not used in this application.
    (void)contactTime;

    // Mesh0 triangles that are intersecting change from blue to cyan.
    auto const& mesh0 = record0.GetMesh();
    std::array<int32_t, 3> indices0{};
    mesh0->GetTriangle(t0, indices0);
    auto const& vbuffer0 = mCylinder0->GetVertexBuffer();
    auto* vertices0 = vbuffer0->Get<Vertex>();
    for (size_t j = 0; j < 3; ++j)
    {
        vertices0[indices0[j]].tcoord = mCyanUV;
    }
    mEngine->Update(vbuffer0);

    // Mesh1 triangles that are intersecting change from red to yellow.
    auto const& mesh1 = record1.GetMesh();
    std::array<int32_t, 3> indices1{};
    mesh1->GetTriangle(t1, indices1);
    auto const& vbuffer1 = mCylinder1->GetVertexBuffer();
    auto* vertices1 = vbuffer1->Get<Vertex>();
    for (size_t j = 0; j < 3; ++j)
    {
        vertices1[indices1[j]].tcoord = mYellowUV;
    }
    mEngine->Update(vbuffer1);
}
