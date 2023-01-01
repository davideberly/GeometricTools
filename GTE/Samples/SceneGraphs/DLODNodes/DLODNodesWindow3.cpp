// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.03.07

#include "DLODNodesWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/PointLightEffect.h>

DLODNodesWindow3::DLODNodesWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mScene{},
    mDLODNode{},
    mCuller{},
    mLightWorldDirection{}
{
    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 1000.0f, 0.001f, 0.001f,
        { 0.0f, -4.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    // The light world direction is the camera view direction.
    mLightWorldDirection = { 0.0f, 1.0f, 0.0f };

    CreateScene();

    mTrackBall.Update();
    mPVWMatrices.Update();
    mCuller.ComputeVisibleSet(mCamera, mScene);
}

void DLODNodesWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
        mCuller.ComputeVisibleSet(mCamera, mScene);
    }

    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mCuller.GetVisibleSet());
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

void DLODNodesWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();
    mTrackBall.Attach(mScene);

    int32_t constexpr numLevelsOfDetail = 6;
    mDLODNode = std::make_shared<DLODNode>(numLevelsOfDetail);
    mScene->AttachChild(mDLODNode);

    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf{};
    mf.SetVertexFormat(vformat);

    // Create a sphere mesh (child 0).
    auto mesh = mf.CreateSphere(32, 16, 1.0f);
    AttachEffect(mesh);

    // Create an icosahedron (child 1).
    mesh = mf.CreateIcosahedron();
    AttachEffect(mesh);

    // Create a dodecahedron (child 2).
    mesh = mf.CreateDodecahedron();
    AttachEffect(mesh);

    // Create an octahedron (child 3).
    mesh = mf.CreateOctahedron();
    AttachEffect(mesh);

    // Create a hexahedron (child 4).
    mesh = mf.CreateHexahedron();
    AttachEffect(mesh);

    // Create a tetrahedron (child 5).
    mesh = mf.CreateTetrahedron();
    AttachEffect(mesh);

    // Set the distance intervals for switching the active child.
    mDLODNode->SetModelDistance(0, 0.0f, 10.0f);
    mDLODNode->SetModelDistance(1, 10.0f, 20.0f);
    mDLODNode->SetModelDistance(2, 20.0f, 30.0f);
    mDLODNode->SetModelDistance(3, 30.0f, 40.0f);
    mDLODNode->SetModelDistance(4, 40.0f, 50.0f);
    mDLODNode->SetModelDistance(5, 50.0f, 100.0f);

    // Set the model level-of-detail center.
    mDLODNode->SetModelLODCenter(Vector4<float>::Zero());
}

void DLODNodesWindow3::AttachEffect(std::shared_ptr<Visual> const& mesh)
{
    auto material = std::make_shared<Material>();
    material->diffuse = { 0.5f, 0.0f, 0.5f, 1.0f };

    auto lighting = std::make_shared<Lighting>();
    lighting->ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    lighting->diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
    lighting->specular = { 0.0f, 0.0f, 0.0f, 0.0f };

    auto geometry = std::make_shared<LightCameraGeometry>();
    geometry->lightModelDirection = mLightWorldDirection;

    auto effect = std::make_shared<PointLightEffect>(mProgramFactory,
        mUpdater, 1, material, lighting, geometry);
    mesh->SetEffect(effect);

    mDLODNode->AttachChild(mesh);
    mPVWMatrices.Subscribe(mesh);
}

void DLODNodesWindow3::UpdateConstants()
{
    // The pvw-matrices are updated automatically whenever the camera moves
    // or the trackball is rotated, which happens before this call. This
    // function updates the shader constants that use the camera model
    // position and light model position.

    // Get the active child of the DLOD node.
    int32_t activeChild = mDLODNode->GetActiveChild();
    if (activeChild == SwitchNode::invalidChild)
    {
        return;
    }
    auto const& node = mDLODNode->GetChild(activeChild);
    auto const& visual = std::dynamic_pointer_cast<Visual>(node);

    // Update the geometry information for the effect.
    Vector4<float> cameraWorldPosition = mCamera->GetPosition();
    auto const& effect = visual->GetEffect();
    auto const& pointLightEffect = std::dynamic_pointer_cast<PointLightEffect>(effect);
    auto const& geometry = pointLightEffect->GetGeometry();
    auto const& invWMatrix = visual->worldTransform.GetHInverse();
    geometry->lightModelPosition = DoTransform(invWMatrix, cameraWorldPosition);
    geometry->cameraModelPosition = DoTransform(invWMatrix, cameraWorldPosition);
    pointLightEffect->UpdateGeometryConstant();
}
