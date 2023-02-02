// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "IKControllersWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/IKController.h>
#include <Graphics/VertexColorEffect.h>

IKControllersWindow3::IKControllersWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.1f, 0.01f,
        { 0.0f, -2.0f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();
    mTrackBall.Update();
    mPVWMatrices.Update();
}

void IKControllersWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    mEngine->Draw(mGround);
    mEngine->Draw(mGoalCube);
    mEngine->Draw(mOriginCube);
    mEngine->Draw(mEndCube);
    mEngine->Draw(mRod);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IKControllersWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    if (Transform(key))
    {
        return true;
    }

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

bool IKControllersWindow3::OnMouseMotion(MouseButton button, int32_t x, int32_t y, uint32_t modifiers)
{
    if (Window3::OnMouseMotion(button, x, y, modifiers))
    {
        UpdateRod();
    }
    return true;
}

void IKControllersWindow3::CreateScene()
{
    // Scene
    //     GroundMesh
    //     IKSystem
    //         Goal
    //             GoalCube
    //         Joint0
    //             OriginCube
    //             Rod
    //             Joint1
    //                 EndCube

    // Create the scene root.
    mScene = std::make_shared<Node>();
    mTrackBall.Attach(mScene);

    // Create the drawable objects.
    mGround = CreateGround();
    mGoalCube = CreateCube();
    mOriginCube = CreateCube();
    mEndCube = CreateCube();
    mRod = CreateRod();

    // Create the IK objects.
    mIKSystem = std::make_shared<Node>();
    mGoal = std::make_shared<Node>();
    mJoint0 = std::make_shared<Node>();
    mJoint1 = std::make_shared<Node>();
    mRod = CreateRod();
    mGoal->localTransform.SetTranslation(0.0f, 2.0f, 0.0f);
    mJoint1->localTransform.SetTranslation(1.0f, 0.0f, 0.0f);

    // Set the parent-child links.
    mScene->AttachChild(mGround);
    mScene->AttachChild(mIKSystem);
    mIKSystem->AttachChild(mGoal);
    mGoal->AttachChild(mGoalCube);
    mIKSystem->AttachChild(mJoint0);
    mJoint0->AttachChild(mOriginCube);
    mJoint0->AttachChild(mRod);
    mJoint0->AttachChild(mJoint1);
    mJoint1->AttachChild(mEndCube);

    // Create the IK controller for the IK system.
    size_t const numJoints = 2;
    size_t const numGoals = 1;
    size_t const numIterations = 1;
    bool const orderEndToRoot = true;
    auto controller = std::make_shared<IKController>(
        numJoints, numGoals, numIterations, orderEndToRoot);

    // Create the goal.
    controller->InitializeGoal(0, mGoal, mJoint1, 1.0f);

    // Create the joints.
    controller->InitializeJoint(0, mJoint0, { 0 });
    controller->SetJointAllowRotation(0, 2, true);
    controller->InitializeJoint(1, mJoint1, { 0 });
    controller->SetJointAllowTranslation(1, 2, true);

    mJoint0->AttachController(controller);

    // Run the IK solver the first time to obtain correct placement
    // of objects in the scene.
    mScene->Update();
    UpdateRod();
}

std::shared_ptr<Visual> IKControllersWindow3::CreateCube()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    float const extent = 0.1f;
    auto cube = mf.CreateBox(extent, extent, extent);
    auto const& vbuffer = cube->GetVertexBuffer();
    auto vertices = vbuffer->Get<Vertex>();
    vertices[0].color = { 0.0f, 0.0f, 1.0f, 1.0f };
    vertices[1].color = { 0.0f, 1.0f, 0.0f, 1.0f };
    vertices[2].color = { 1.0f, 0.0f, 0.0f, 1.0f };
    vertices[3].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    vertices[4].color = { 0.0f, 0.0f, 1.0f, 1.0f };
    vertices[5].color = { 1.0f, 0.0f, 1.0f, 1.0f };
    vertices[6].color = { 1.0f, 1.0f, 0.0f, 1.0f };
    vertices[7].color = { 1.0f, 1.0f, 1.0f, 1.0f };

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    cube->SetEffect(effect);
    mPVWMatrices.Subscribe(cube->worldTransform, effect->GetPVWMatrixConstant());
    return cube;
}

std::shared_ptr<Visual> IKControllersWindow3::CreateRod()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto vertices = vbuffer->Get<Vertex>();

    vertices[0].position = { 0.0f, 0.0f, 0.0f };
    vertices[1].position = { 1.0f, 0.0f, 0.0f };
    vertices[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    vertices[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);

    auto segment = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(segment->worldTransform, effect->GetPVWMatrixConstant());
    return segment;
}

std::shared_ptr<Visual> IKControllersWindow3::CreateGround()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    float const extent = 16.0f;
    auto plane = mf.CreateRectangle(2, 2, extent, extent);
    auto const& vbuffer = plane->GetVertexBuffer();
    auto vertices = vbuffer->Get<Vertex>();
    float const z = -0.1f;
    vertices[0].position[2] = z;
    vertices[1].position[2] = z;
    vertices[2].position[2] = z;
    vertices[3].position[2] = z;
    vertices[0].color = { 0.5f, 0.5f, 0.70f, 1.0f };
    vertices[1].color = { 0.5f, 0.5f, 0.80f, 1.0f };
    vertices[2].color = { 0.5f, 0.5f, 0.90f, 1.0f };
    vertices[3].color = { 0.5f, 0.5f, 1.00f, 1.0f };

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    plane->SetEffect(effect);
    mPVWMatrices.Subscribe(plane->worldTransform, effect->GetPVWMatrixConstant());
    return plane;
}

void IKControllersWindow3::UpdateRod()
{
    // The vertex[0] never moves.  The rod mesh is in the coordinate system
    // of joint0, so use the local translation of joint1 for the rod mesh's
    // moving end point.
    auto const& vbuffer = mRod->GetVertexBuffer();
    auto vertices = vbuffer->Get<Vertex>();
    vertices[1].position = mJoint1->localTransform.GetTranslation();
    mEngine->Update(vbuffer);
}

bool IKControllersWindow3::Transform(uint8_t key)
{
    Matrix4x4<float> rot, incr;
    Vector3<float> trn;
    float const trnSpeed = mCameraRig.GetTranslationSpeed();

    switch (key)
    {
    case 'x':
        trn = mGoal->localTransform.GetTranslation();
        trn[0] -= trnSpeed;
        mGoal->localTransform.SetTranslation(trn);
        break;
    case 'X':
        trn = mGoal->localTransform.GetTranslation();
        trn[0] += trnSpeed;
        mGoal->localTransform.SetTranslation(trn);
        break;
    case 'y':
        trn = mGoal->localTransform.GetTranslation();
        trn[1] -= trnSpeed;
        mGoal->localTransform.SetTranslation(trn);
        break;
    case 'Y':
        trn = mGoal->localTransform.GetTranslation();
        trn[1] += trnSpeed;
        mGoal->localTransform.SetTranslation(trn);
        break;
    case 'z':
        trn = mGoal->localTransform.GetTranslation();
        trn[2] -= trnSpeed;
        mGoal->localTransform.SetTranslation(trn);
        break;
    case 'Z':
        trn = mGoal->localTransform.GetTranslation();
        trn[2] += trnSpeed;
        mGoal->localTransform.SetTranslation(trn);
        break;
    default:
        return false;
    }

    mIKSystem->Update();
    UpdateRod();
    mPVWMatrices.Update();
    return true;
}
