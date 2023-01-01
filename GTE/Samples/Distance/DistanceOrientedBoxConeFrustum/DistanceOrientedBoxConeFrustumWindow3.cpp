// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "DistanceOrientedBoxConeFrustumWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/VertexColorEffect.h>
#include <Mathematics/Rotation.h>
#include <Mathematics/Minimize1.h>

DistanceOrientedBoxConeFrustumWindow3::DistanceOrientedBoxConeFrustumWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mVFormat{},
    mDRE{},
    mURD(0.25, 0.75f),
    mBox{},
    mCone{},
    mQuadrilateral{},
    mQuadAngle(0.0f),
    mBoxQuadDistance(0.0f),
    mBoxConeDistance(0.0f),
    mBoxClosestToQuad{},
    mBoxClosestToCone{},
    mQuadClosest{},
    mConeClosest{},
    mBoxQuadQuery{},
    mBoxConeQuery{}
{
    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->fill = RasterizerState::Fill::SOLID;
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mEngine->SetRasterizerState(mNoCullState);

    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.001f, 0.001f,
        { 12.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();

    Update();
    mTrackBall.Update();
    mPVWMatrices.Update();
}

void DistanceOrientedBoxConeFrustumWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    std::shared_ptr<RasterizerState> save = mEngine->GetRasterizerState();
    mEngine->SetRasterizerState(mNoCullState);
    mEngine->Draw(mQuadMesh);
    mEngine->Draw(mBoxQuadSegmentMesh);
    mEngine->Draw(mBoxConeSegmentMesh);
    mEngine->Draw(mBoxClosestToQuadMesh);
    mEngine->Draw(mQuadClosestMesh);
    mEngine->Draw(mBoxClosestToConeMesh);
    mEngine->Draw(mConeClosestMesh);
    mEngine->SetRasterizerState(save);

    mEngine->Draw(mBoxMesh);
    mEngine->Draw(mConeMesh);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool DistanceOrientedBoxConeFrustumWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    float const deltaAngle = (float)(GTE_C_PI / 180.0);
    float const deltaTranslate = 0.1f;

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

    case '-':
    case '_':
        mQuadAngle -= deltaAngle;
        Update();
        return true;

    case '+':
    case '=':
        mQuadAngle += deltaAngle;
        Update();
        return true;

    case 'x':  // decrement x-center of box
        Translate(0, -deltaTranslate);
        return true;

    case 'X':  // increment x-center of box
        Translate(0, +deltaTranslate);
        return true;

    case 'y':  // decrement y-center of box
        Translate(1, -deltaTranslate);
        return true;

    case 'Y':  // increment y-center of box
        Translate(1, +deltaTranslate);
        return true;

    case 'z':  // decrement z-center of box
        Translate(2, -deltaTranslate);
        return true;

    case 'Z':  // increment z-center of box
        Translate(2, +deltaTranslate);
        return true;

    case 'p':  // rotate about axis[0]
        Rotate(0, -deltaTranslate);
        return true;

    case 'P':  // rotate about axis[0]
        Rotate(0, +deltaTranslate);
        return true;

    case 'r':  // rotate about axis[1]
        Rotate(1, -deltaTranslate);
        return true;

    case 'R':  // rotate about axis[1]
        Rotate(1, +deltaTranslate);
        return true;

    case 'h':  // rotate about axis[2]
        Rotate(2, -deltaTranslate);
        return true;

    case 'H':  // rotate about axis[2]
        Rotate(2, +deltaTranslate);
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

void DistanceOrientedBoxConeFrustumWindow3::CreateScene()
{
    Ray<3, float> ray{};
    ray.origin = { -1.0f, -1.0f, -1.0f };
    ray.direction = { 1.0f, 2.0f, 3.0f };
    Normalize(ray.direction);
    float angle = static_cast<float>(0.6);
    float hmin = 0.5f;
    float hmax = 3.0f;
    mCone = Cone<3, float>(ray, angle, hmin, hmax);

    mBox.center = ray.origin + 0.5f * (hmin + hmax) * ray.direction +
        Vector<3, float>{ 4.0f, -2.0f, 0.0f };
    Quaternion<float> q{ 4.0f, 3.0f, 2.0f, 1.0f };
    Normalize(q);
    Matrix<3, 3, float> R = Rotation<3, float>(q);
    mBox.axis[0] = R.GetCol(0);
    mBox.axis[1] = R.GetCol(1);
    mBox.axis[2] = R.GetCol(2);
    mBox.extent = { 3.0f, 2.0f, 1.0f };
    mBox.extent *= 0.25f;

    mQuadAngle = 0.0f;
    mBoxQuadDistance = std::numeric_limits<float>::max();
    mBoxConeDistance = std::numeric_limits<float>::max();
    mBoxClosestToQuad.MakeZero();
    mBoxClosestToCone.MakeZero();
    mQuadClosest.MakeZero();
    mConeClosest.MakeZero();
    (void)mBoxQuadQuery(mBox, mCone, mQuadAngle, mBoxQuadDistance, mBoxClosestToQuad, mQuadClosest);

    auto bcResult = mBoxConeQuery(mBox, mCone);
    mBoxConeDistance = bcResult.distance;
    mBoxClosestToCone = bcResult.boxClosestPoint;
    mConeClosest = bcResult.coneClosestPoint;

    mVFormat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    mVFormat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);

    CreateBoxMesh();
    CreateConeMesh();
    CreateQuadMesh();
    CreateSegmentMeshes();
    CreateClosestPointMeshes();
}

void DistanceOrientedBoxConeFrustumWindow3::CreateBoxMesh()
{
    std::array<Vector<3, float>, 8> corners{};
    mBox.GetVertices(corners);
    auto vbuffer = std::make_shared<VertexBuffer>(mVFormat, 8);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto* vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < 8; ++i)
    {
        vertices[i].position = corners[i];
        vertices[i].color = { mURD(mDRE), 0.0f, mURD(mDRE), 1.0f };
    }

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 12, sizeof(int32_t));
    auto* indices = ibuffer->Get<int32_t>();
    indices[0] = 0; indices[1] = 4; indices[2] = 6;
    indices[3] = 0; indices[4] = 6; indices[5] = 2;
    indices[6] = 1; indices[7] = 3; indices[8] = 7;
    indices[9] = 1; indices[10] = 7; indices[11] = 5;
    indices[12] = 0; indices[13] = 1; indices[14] = 5;
    indices[15] = 0; indices[16] = 5; indices[17] = 4;
    indices[18] = 2; indices[19] = 6; indices[20] = 7;
    indices[21] = 2; indices[22] = 7; indices[23] = 3;
    indices[24] = 0; indices[25] = 2; indices[26] = 3;
    indices[27] = 0; indices[28] = 3; indices[29] = 1;
    indices[30] = 4; indices[31] = 5; indices[32] = 7;
    indices[33] = 4; indices[34] = 7; indices[35] = 6;

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);

    mBoxMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mBoxMesh);
    mTrackBall.Attach(mBoxMesh);
}

void DistanceOrientedBoxConeFrustumWindow3::CreateConeMesh()
{
    std::vector<Vector<3, float>> coneVertices;
    std::vector<int32_t> coneIndices;
    mCone.CreateMesh(32, true, coneVertices, coneIndices);
    uint32_t numVertices = static_cast<uint32_t>(coneVertices.size());
    auto vbuffer = std::make_shared<VertexBuffer>(mVFormat, numVertices);
    auto* vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].position = coneVertices[i];
        vertices[i].color = { 0.0f, mURD(mDRE), mURD(mDRE), 1.0f };
    }

    uint32_t numTriangles = static_cast<uint32_t>(coneIndices.size() / 3);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(int32_t));
    auto* indices = ibuffer->Get<int32_t>();
    std::memcpy(indices, coneIndices.data(), coneIndices.size() * sizeof(int32_t));

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);

    mConeMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mConeMesh);
    mTrackBall.Attach(mConeMesh);
}

void DistanceOrientedBoxConeFrustumWindow3::CreateQuadMesh()
{
    auto vbuffer = std::make_shared<VertexBuffer>(mVFormat, 4);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto* vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < 4; ++i)
    {
        vertices[i].position = mBoxQuadQuery.quadrilateral[i];
        vertices[i].color = { mURD(mDRE), mURD(mDRE), 0.0f, 1.0f };
    }

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(int32_t));
    auto* indices = ibuffer->Get<int32_t>();
    indices[0] = 0; indices[1] = 1; indices[2] = 3;
    indices[3] = 0; indices[4] = 3; indices[5] = 2;

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);

    mQuadMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mQuadMesh);
    mTrackBall.Attach(mQuadMesh);
}

void DistanceOrientedBoxConeFrustumWindow3::CreateSegmentMeshes()
{
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 2);

    auto vbuffer = std::make_shared<VertexBuffer>(mVFormat, 2);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto* vertices = vbuffer->Get<Vertex>();
    vertices[0].position = mBoxClosestToQuad;
    vertices[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    vertices[1].position = mQuadClosest;
    vertices[1].color = vertices[0].color;
    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mBoxQuadSegmentMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mBoxQuadSegmentMesh);
    mTrackBall.Attach(mBoxQuadSegmentMesh);

    vbuffer = std::make_shared<VertexBuffer>(mVFormat, 2);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    vertices = vbuffer->Get<Vertex>();
    vertices[0].position = mBoxClosestToCone;
    vertices[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    vertices[1].position = mConeClosest;
    vertices[1].color = vertices[0].color;
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mBoxConeSegmentMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mBoxConeSegmentMesh);
    mTrackBall.Attach(mBoxConeSegmentMesh);
}

void DistanceOrientedBoxConeFrustumWindow3::CreateClosestPointMeshes()
{
    MeshFactory mf;
    mf.SetVertexFormat(mVFormat);

    uint32_t numZSamples = 8;
    uint32_t numRadialSamples = 8;
    float radius = 0.0375f;
    Vector<4, float> black{ 0.0f, 0.0f, 0.0f, 1.0f };

    mBoxClosestToQuadMesh = mf.CreateSphere(numZSamples, numRadialSamples, radius);
    std::shared_ptr<VertexBuffer> vbuffer = mBoxClosestToQuadMesh->GetVertexBuffer();
    uint32_t numVertices = vbuffer->GetNumElements();
    auto* vertices = vbuffer->Get<Vertex>();
    for (size_t i = 0; i < numVertices; ++i)
    {
        vertices[i].color = black;
    }
    mBoxClosestToQuadMesh->localTransform.SetTranslation(mBoxClosestToQuad);
    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mBoxClosestToQuadMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mBoxClosestToQuadMesh);
    mTrackBall.Attach(mBoxClosestToQuadMesh);

    mQuadClosestMesh = mf.CreateSphere(numZSamples, numRadialSamples, radius);
    vbuffer = mQuadClosestMesh->GetVertexBuffer();
    numVertices = vbuffer->GetNumElements();
    vertices = vbuffer->Get<Vertex>();
    for (size_t i = 0; i < numVertices; ++i)
    {
        vertices[i].color = black;
    }
    mQuadClosestMesh->localTransform.SetTranslation(mQuadClosest);
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mQuadClosestMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mQuadClosestMesh);
    mTrackBall.Attach(mQuadClosestMesh);

    mBoxClosestToConeMesh = mf.CreateSphere(numZSamples, numRadialSamples, radius);
    vbuffer = mBoxClosestToConeMesh->GetVertexBuffer();
    numVertices = vbuffer->GetNumElements();
    vertices = vbuffer->Get<Vertex>();
    for (size_t i = 0; i < numVertices; ++i)
    {
        vertices[i].color = black;
    }
    mBoxClosestToConeMesh->localTransform.SetTranslation(mBoxClosestToCone);
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mBoxClosestToConeMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mBoxClosestToConeMesh);
    mTrackBall.Attach(mBoxClosestToConeMesh);

    mConeClosestMesh = mf.CreateSphere(numZSamples, numRadialSamples, radius);
    vbuffer = mConeClosestMesh->GetVertexBuffer();
    numVertices = vbuffer->GetNumElements();
    vertices = vbuffer->Get<Vertex>();
    for (size_t i = 0; i < numVertices; ++i)
    {
        vertices[i].color = black;
    }
    mConeClosestMesh->localTransform.SetTranslation(mConeClosest);
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mConeClosestMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mConeClosestMesh);
    mTrackBall.Attach(mConeClosestMesh);
}

void DistanceOrientedBoxConeFrustumWindow3::Translate(int32_t direction, float delta)
{
    mBox.center[direction] += delta;

    auto bcResult = mBoxConeQuery(mBox, mCone);
    mBoxConeDistance = bcResult.distance;
    mBoxClosestToCone = bcResult.boxClosestPoint;
    mConeClosest = bcResult.coneClosestPoint;

    std::shared_ptr<VertexBuffer> vbuffer = mBoxMesh->GetVertexBuffer();
    auto* vertices = vbuffer->Get<Vertex>();
    std::array<Vector<3, float>, 8> corners{};
    mBox.GetVertices(corners);
    for (uint32_t i = 0; i < 8; ++i)
    {
        vertices[i].position = corners[i];
    }

    mEngine->Update(vbuffer);
    Update();
}

void DistanceOrientedBoxConeFrustumWindow3::Rotate(int32_t direction, float delta)
{
    AxisAngle<3, float> aa(mBox.axis[direction], delta);
    Quaternion<float> incr = Rotation<3, float>(aa);
    for (int32_t i = 0; i < 3; ++i)
    {
        if (i != direction)
        {
            mBox.axis[i] = HProject(gte::Rotate(incr, HLift(mBox.axis[i], 0.0f)));
        }
    }

    auto bcResult = mBoxConeQuery(mBox, mCone);
    mBoxConeDistance = bcResult.distance;
    mBoxClosestToCone = bcResult.boxClosestPoint;
    mConeClosest = bcResult.coneClosestPoint;

    std::shared_ptr<VertexBuffer> vbuffer = mBoxMesh->GetVertexBuffer();
    auto* vertices = vbuffer->Get<Vertex>();
    std::array<Vector<3, float>, 8> corners{};
    mBox.GetVertices(corners);
    for (uint32_t i = 0; i < 8; ++i)
    {
        vertices[i].position = corners[i];
    }

    mEngine->Update(vbuffer);
    Update();
}

void DistanceOrientedBoxConeFrustumWindow3::Update()
{
    mBoxQuadDistance = -1.0f;
    mBoxClosestToQuad.MakeZero();
    mQuadClosest.MakeZero();
    (void)mBoxQuadQuery(mBox, mCone, mQuadAngle, mBoxQuadDistance, mBoxClosestToQuad, mQuadClosest);

    std::array<Vector<3, float>, 3> basis{};
    basis[0] = mCone.ray.direction;
    ComputeOrthogonalComplement(1, basis.data());
    float cs = std::cos(mQuadAngle);
    float sn = std::sin(mQuadAngle);
    Vector<3, float> term = mCone.tanAngle * (cs * basis[1] + sn * basis[2]);
    std::array<Vector<3, float>, 2> G{};
    G[0] = mCone.ray.direction - term;
    G[1] = mCone.ray.direction + term;
    float const hmin = mCone.GetMinHeight();
    float const hmax = mCone.GetMaxHeight();
    mQuadrilateral[0] = mCone.ray.origin + hmin * G[0];
    mQuadrilateral[1] = mCone.ray.origin + hmin * G[1];
    mQuadrilateral[2] = mCone.ray.origin + hmax * G[0];
    mQuadrilateral[3] = mCone.ray.origin + hmax * G[1];

    std::shared_ptr<VertexBuffer> vbuffer = mQuadMesh->GetVertexBuffer();
    auto* vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < 4; ++i)
    {
        vertices[i].position = mQuadrilateral[i];
    }
    mEngine->Update(vbuffer);

    vbuffer = mBoxQuadSegmentMesh->GetVertexBuffer();
    vertices = vbuffer->Get<Vertex>();
    vertices[0].position = mBoxClosestToQuad;
    vertices[1].position = mQuadClosest;
    mEngine->Update(vbuffer);

    vbuffer = mBoxConeSegmentMesh->GetVertexBuffer();
    vertices = vbuffer->Get<Vertex>();
    vertices[0].position = mBoxClosestToCone;
    vertices[1].position = mConeClosest;
    mEngine->Update(vbuffer);

    mBoxClosestToQuadMesh->localTransform.SetTranslation(mBoxClosestToQuad);
    mBoxClosestToConeMesh->localTransform.SetTranslation(mBoxClosestToCone);
    mQuadClosestMesh->localTransform.SetTranslation(mQuadClosest);
    mConeClosestMesh->localTransform.SetTranslation(mConeClosest);
    mTrackBall.Update();
    mPVWMatrices.Update();
}
