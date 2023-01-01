// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.21

#include "CollisionsMovingSphereTriangleWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/VertexColorEffect.h>
#include <random>

CollisionsMovingSphereTriangleWindow3::CollisionsMovingSphereTriangleWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mNoCullState{},
    mNoCullWireState{},
    mSphereMesh{},
    mTriangleMesh{},
    mContactMesh{},
    mCenters{},
    mVisuals{},
    mMotionObject{},
    mSphere{},
    mTriangle{},
    mSphereVelocity(Vector3<float>::Zero()),
    mTriangleVelocity(Vector3<float>::Zero()),
    mSimulationTime(0.0f),
    mSimulationDeltaTime(0.0f),
    mMTri{ Vector3<float>::Zero(), Vector3<float>::Zero(), Vector3<float>::Zero() },
    mUseInitialCenter(true)
{
    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetRasterizerState(mNoCullState);
    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f });

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 8.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();
    Update();
}

void CollisionsMovingSphereTriangleWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    Update();

    mEngine->ClearBuffers();
    mEngine->Draw(mVisuals);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool CollisionsMovingSphereTriangleWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
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

    case '0':
        mMotionObject = mScene;
        return true;

    case '1':
        mMotionObject = mSphereMesh;
        return true;

    case '2':
        mMotionObject = mTriangleMesh;
        return true;

    case ' ':
        // Toggle the sphere mesh, not the actual sphere, between the
        // initial center and the center when in contact with triangle.
        mUseInitialCenter = !mUseInitialCenter;
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

bool CollisionsMovingSphereTriangleWindow3::OnMouseClick(MouseButton button, MouseState state, int32_t x, int32_t y, uint32_t)
{
    if (button == MOUSE_LEFT)
    {
        if (state == MOUSE_DOWN)
        {
            auto const& root = mTrackBall.GetRoot();
            root->localTransform.SetRotation(
                mMotionObject->localTransform.GetRotation());

            mTrackBall.SetActive(true);
            mTrackBall.SetInitialPoint(x, mYSize - 1 - y);
        }
        else
        {
            mTrackBall.SetActive(false);
        }

        return true;
    }

    return false;
}

bool CollisionsMovingSphereTriangleWindow3::OnMouseMotion(MouseButton button,
    int32_t x, int32_t y, uint32_t)
{
    if (button == MOUSE_LEFT && mTrackBall.GetActive())
    {
        mTrackBall.SetFinalPoint(x, mYSize - 1 - y);

        mMotionObject->localTransform.SetRotation(mTrackBall.GetOrientation());
        Update();
        return true;
    }

    return false;
}

void CollisionsMovingSphereTriangleWindow3::CreateScene()
{
    mSimulationTime = 0.001f;
    mSimulationDeltaTime = 0.001f;
    mMTri[0] = { 0.0f, 0.0f, 0.0f };
    mMTri[1] = { 0.0f, 1.0f, 0.0f };
    mMTri[2] = { -0.6f, 0.7f, 0.8f };
    Vector3<float> average = (mMTri[0] + mMTri[1] + mMTri[2]) / 3.0f;
    mMTri[0] -= average;
    mMTri[1] -= average;
    mMTri[2] -= average;
    mTriangle.position[0] = mMTri[0];
    mTriangle.position[1] = mMTri[1];
    mTriangle.position[2] = mMTri[2];
    mTriangle.ComputeDerived();
    mTriangleVelocity = { 0.0f, 0.0f, 0.0f };
    mSphere.center = { 0.0f, 0.0f, 2.0f };
    mSphere.radius = 0.3f;
    mSphere.ComputeDerived();
    mSphereVelocity = { 0.0f, 0.0f, -1.0f },
    mUseInitialCenter = true;

    std::default_random_engine dre{};
    std::uniform_real_distribution<float> urd(0.0f, 1.0f);

    mScene = std::make_shared<Node>();
    mVisuals.resize(4);

    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    MeshFactory mf{};
    mf.SetVertexFormat(vformat);

    mSphereMesh = mf.CreateSphere(16, 16, mSphere.radius);
    std::shared_ptr<VertexBuffer> vbuffer = mSphereMesh->GetVertexBuffer();
    auto* vertices = vbuffer->Get<Vertex>();
    uint32_t numVertices = vbuffer->GetNumElements();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].color = { urd(dre), 0.0f, 0.0f, 1.0f };
    }
    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mSphereMesh->SetEffect(effect);

    // Orient the sphere so that the first column of the local rotation
    // matrix is the sphere velocity direction.
    Matrix3x3<float> rotate{};
    rotate.SetCol(0, Vector3<float>{ 0.0f, 0.0f, -1.0f });
    rotate.SetCol(1, Vector3<float>{ 0.0f, 1.0f, 0.0f });
    rotate.SetCol(2, Vector3<float>{ 1.0f, 0.0f, 0.0f });
    mSphereMesh->localTransform.SetTranslation(mSphere.center);
    mSphereMesh->localTransform.SetRotation(rotate);
    mPVWMatrices.Subscribe(mSphereMesh);
    mScene->AttachChild(mSphereMesh);
    mVisuals[0] = mSphereMesh;

    vbuffer = std::make_shared<VertexBuffer>(vformat, 3);
    vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < 3; ++i)
    {
        vertices[i].position = mTriangle.position[i];
        vertices[i].color = { 0.0f, 0.0f, urd(dre), 1.0f };
    }
    auto ibuffer = std::make_shared<IndexBuffer>(IPType::IP_TRIMESH, 1);
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mTriangleMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mTriangleMesh);
    mScene->AttachChild(mTriangleMesh);
    mVisuals[1] = mTriangleMesh;

    vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    vertices = vbuffer->Get<Vertex>();
    vertices[0].position = mSphere.center;
    vertices[0].color = { 1.0f, 1.0f, 0.0f, 1.0f };
    vertices[1].position = mSphere.center + 100.0f * mSphereVelocity;
    vertices[1].color = { 1.0f, 1.0f, 0.0f, 1.0f };
    ibuffer = std::make_shared<IndexBuffer>(IPType::IP_POLYSEGMENT_DISJOINT, 1);
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mCenters = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mCenters);
    mScene->AttachChild(mCenters);
    mVisuals[2] = mCenters;

    mContactMesh = mf.CreateSphere(16, 16, 0.05f);
    vbuffer = mContactMesh->GetVertexBuffer();
    vertices = vbuffer->Get<Vertex>();
    numVertices = vbuffer->GetNumElements();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].color = { 0.0f, 1.0f, 0.0f, 1.0f };
    }
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mContactMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mContactMesh);
    mScene->AttachChild(mContactMesh);
    mVisuals[3] = mContactMesh;

    mMotionObject = mScene;
}

void CollisionsMovingSphereTriangleWindow3::Update()
{
    // Update the sphere and triangle based on how they were moved by the
    // user (during an OnMotion operation).
    Matrix3x3<float> rotate{};
    mTriangleMesh->localTransform.GetRotation(rotate);
    for (size_t i = 0; i < 3; ++i)
    {
        mTriangle.position[i] = rotate * mMTri[i];
    }
    mTriangle.ComputeDerived();

    mSphereMesh->localTransform.GetRotation(rotate);
    mSphereVelocity = rotate.GetCol(0);

    auto const& vbuffer = mCenters->GetVertexBuffer();
    auto* vertices = vbuffer->Get<Vertex>();
    vertices[1].position = mSphere.center + 100.0f * mSphereVelocity;
    mEngine->Update(vbuffer);

    RTSphereTriangle::ContactType type = RTSphereTriangle::Collide(
        mSphere, mSphereVelocity, mTriangle, mTriangleVelocity, 100.0f,
        mContactTime, mContactPoint);
    if (type != RTSphereTriangle::ContactType::CONTACT)
    {
        // "Hide" the contact by moving it far away.
        mContactPoint = { 1000.0f, 1000.0f, 1000.0f };
    }

    mContactMesh->localTransform.SetTranslation(mContactPoint);

    if (mUseInitialCenter)
    {
        mSphereMesh->localTransform.SetTranslation(mSphere.center);
    }
    else
    {
        mSphereMesh->localTransform.SetTranslation(mSphere.center +
            mContactTime * mSphereVelocity);
    }

    mScene->Update();
    mPVWMatrices.Update();
}
