// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "CameraAndLightNodesWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>
#include <Graphics/Light.h>

CameraAndLightNodesWindow3::CameraAndLightNodesWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;

    mNoDepthStencilState = std::make_shared<DepthStencilState>();
    mNoDepthStencilState->depthEnable = false;
    mNoDepthStencilState->stencilEnable = false;

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    std::string path = mEnvironment.GetPath("RedSky.png");
    auto skyTexture = WICFileIO::Load(path, false);
    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize, mYSize, mXSize, mYSize,
        SamplerState::Filter::MIN_P_MAG_P_MIP_P, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP, true);
    mOverlay->SetTexture(skyTexture);

    // The order of these calls is important.  The camera and camera node must
    // be created first so that the camera node has the correct transforms
    // initially.  The camera node is then attached to the scene and the update
    // works correctly.
    InitializeCameraNode();
    CreateScene();

    mScene->Update();
    mPVWMatrices.Update();
}

void CameraAndLightNodesWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraNodeRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    // Draw the red-sky background.
    mEngine->SetDepthStencilState(mNoDepthStencilState);
    mEngine->Draw(mOverlay);
    mEngine->SetDefaultDepthStencilState();

    // Draw the ground and light disks.
    mEngine->Draw(mGround);
    mEngine->SetBlendState(mBlendState);
    mEngine->Draw(mLightTarget[0]);
    mEngine->Draw(mLightTarget[1]);
    mEngine->SetDefaultBlendState();

    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 1.0f, 1.0f }, mTimer.GetFPS());

    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool CameraAndLightNodesWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
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

    case '+':  // increase light intensity
    case '=':
        for (int32_t i = 0; i < 2; ++i)
        {
            auto const& lighting = mEffect[i]->GetLighting();
            lighting->attenuation[3] += 0.1f;
            mEffect[i]->UpdateLightingConstant();
        }
        return true;

    case '-':  // decrease light intensity
    case '_':
        if (mEffect[0]->GetLighting()->attenuation[3] >= 0.1f)
        {
            for (int32_t i = 0; i < 2; ++i)
            {
                auto const& lighting = mEffect[i]->GetLighting();
                lighting->attenuation[3] -= 0.1f;
                mEffect[i]->UpdateLightingConstant();
            }
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool CameraAndLightNodesWindow3::OnKeyDown(int32_t key, int32_t, int32_t)
{
    return mCameraNodeRig.PushMotion(key);
}

bool CameraAndLightNodesWindow3::OnKeyUp(int32_t key, int32_t, int32_t)
{
    return mCameraNodeRig.PopMotion(key);
}

bool CameraAndLightNodesWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "Gravel.png",
        "RedSky.png"
    };

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    return true;
}

void CameraAndLightNodesWindow3::InitializeCameraNode()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 1.0f, 1000.0f);
    Vector4<float> camPosition{ 0.0f, -100.0f, 5.0f, 1.0f };
    Vector4<float> camDVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mCameraNode = std::make_shared<ViewVolumeNode>(mCamera);
    mCameraNode->SetOnUpdate([this](ViewVolumeNode* cameraNode)
    {
        auto const& invWMatrix = mGround->worldTransform.GetHInverse();
        auto const& cameraWorldPosition = cameraNode->GetViewVolume()->GetPosition();
        mCameraModelPosition = DoTransform(invWMatrix, cameraWorldPosition);
    });
    mCameraNodeRig.SetCameraNode(mCameraNode);

    mCameraNodeRig.Set(mCamera, 0.01f, 0.001f);
    mCameraNodeRig.RegisterMoveForward(KEY_UP);
    mCameraNodeRig.RegisterMoveBackward(KEY_DOWN);
    mCameraNodeRig.RegisterTurnRight(KEY_RIGHT);
    mCameraNodeRig.RegisterTurnLeft(KEY_LEFT);
}

void CameraAndLightNodesWindow3::CreateScene()
{
    // scene -+--> groundPoly
    //        |
    //        +--> cameraNode --+--> lightFixture0 +--> lightNode0
    //                          |                  |
    //                          |                  +--> lightTarget0
    //                          |
    //                          +--> lightFixture1 +--> lightNode1
    //                                             |
    //                                             +--> lightTarget0

    mScene = std::make_shared<Node>();
    mScene->AttachChild(CreateGround());
    mScene->AttachChild(mCameraNode);

    AxisAngle<4, float> aa(Vector4<float>::Unit(0), (float)-GTE_C_HALF_PI);
    auto lightFixture0 = CreateLightFixture(0);
    lightFixture0->localTransform.SetTranslation(25.0f, -5.75f, 6.0f);
    lightFixture0->localTransform.SetRotation(aa);
    mCameraNode->AttachChild(lightFixture0);

    auto lightFixture1 = CreateLightFixture(1);
    lightFixture1->localTransform.SetTranslation(25.0f, -5.75f, -6.0f);
    lightFixture1->localTransform.SetRotation(aa);
    mCameraNode->AttachChild(lightFixture1);
}

std::shared_ptr<Visual> CameraAndLightNodesWindow3::CreateGround()
{
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    auto* vertices = vbuffer->Get<Vertex>();
    vertices[0].position = { -100.0f, -100.0f, 0.0f };
    vertices[1].position = { +100.0f, -100.0f, 0.0f };
    vertices[2].position = { +100.0f, +100.0f, 0.0f };
    vertices[3].position = { -100.0f, +100.0f, 0.0f };
    vertices[0].tcoord = { 0.0f, 0.0f };
    vertices[1].tcoord = { 8.0f, 0.0f };
    vertices[2].tcoord = { 8.0f, 8.0f };
    vertices[3].tcoord = { 0.0f, 8.0f };

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(uint32_t));
    ibuffer->SetTriangle(0, 0, 1, 2);
    ibuffer->SetTriangle(1, 0, 2, 3);

    std::string path = mEnvironment.GetPath("Gravel.png");
    auto gravelTexture = WICFileIO::Load(path, true);
    gravelTexture->AutogenerateMipmaps();

    // Darken the gravel.
    auto* texels = gravelTexture->Get<uint32_t>();
    for (uint32_t i = 0; i < gravelTexture->GetNumElements(); ++i)
    {
        uint32_t r = static_cast<uint32_t>(0.2f * (texels[i] & 0x000000FF));
        uint32_t g = static_cast<uint32_t>(0.2f * ((texels[i] & 0x0000FF00) >> 8));
        uint32_t b = static_cast<uint32_t>(0.2f * ((texels[i] & 0x00FF0000) >> 16));
        texels[i] = r | (g << 8) | (b << 16) | 0xFF000000;
    }

    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, gravelTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mGround = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mGround->worldTransform, effect->GetPVWMatrixConstant());
    return mGround;
}

std::shared_ptr<Node> CameraAndLightNodesWindow3::CreateLightFixture(int32_t i)
{
    auto lightFixture = std::make_shared<Node>();

    // A point light illuminates the target.  Create a semitransparent
    // material for the patch.
    auto material = std::make_shared<Material>();
    auto geometry = std::make_shared<LightCameraGeometry>();
    auto light = std::make_shared<Light>(true, mEngine->HasDepthRange01());
    light->lighting = std::make_shared<Lighting>();

    material->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    material->ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    material->diffuse = { 1.0f, 0.85f, 0.75f, 0.5f };
    material->specular = { 0.8f, 0.8f, 0.8f, 1.0f };
    light->lighting->ambient = { 1.0f, 1.0f, 0.5f, 1.0f };
    light->lighting->diffuse = { 1.0f, 1.0f, 0.5f, 1.0f };
    light->lighting->specular = { 1.0f, 1.0f, 0.5f, 1.0f };
    light->SetPosition({ 0.0f, 0.0f, 0.0f, 1.0f });

    // Create the target itself.  The incoming light is a point light, so use
    // an effect that combines this light with the specified material.
    mLightTarget[i] = CreateLightTarget();

    mEffect[i] = std::make_shared<PointLightEffect>(
        mProgramFactory, mUpdater, 0, material, light->lighting, geometry);
    mLightTarget[i]->SetEffect(mEffect[i]);

    mPVWMatrices.Subscribe(mLightTarget[i]->worldTransform, mEffect[i]->GetPVWMatrixConstant());

    // Encapsulate the light in a light node.  Rotate the light node so the
    // light points downward.
    mLightNode[i] = std::make_shared<ViewVolumeNode>(light);
    mLightNode[i]->SetOnUpdate([this, i](ViewVolumeNode*)
    {
        // The camera model position must be updated for the light targets to
        // move.  The light model position is not updated because the point
        // lights must move with their corresponding light targets.
        auto const& geometry = mEffect[i]->GetGeometry();
        geometry->cameraModelPosition = mCameraModelPosition;
        mEffect[i]->UpdateGeometryConstant();
    });

    lightFixture->AttachChild(mLightNode[i]);
    lightFixture->AttachChild(mLightTarget[i]);
    return lightFixture;
}

std::shared_ptr<Visual> CameraAndLightNodesWindow3::CreateLightTarget()
{
    // Create a parabolic rectangle patch that is illuminated by the light.
    // To hide the artifacts of vertex normal lighting on a grid, the patch
    // is slightly bent so that the intersection with a plane is nearly
    // circular.  The patch is translated slightly below the plane of the
    // ground to hide the corners and the jaggies.
    struct Vertex
    {
        Vector3<float> position, normal;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0);

    // Create a flat surface.
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    auto mesh = mf.CreateRectangle(64, 64, 8.0f, 8.0f);

    // Adjust the heights to form a paraboloid.
    auto const& vbuffer = mesh->GetVertexBuffer();
    uint32_t const numVertices = vbuffer->GetNumActiveElements();
    auto* vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        Vector3<float>& pos = vertices[i].position;
        pos[2] = 1.0f - (pos[0] * pos[0] + pos[1] * pos[1]) / 128.0f;
    }
    mesh->UpdateModelNormals();

    return mesh;
}

void CameraAndLightNodesWindow3::CameraNodeRig::SetCameraNode(
    std::shared_ptr<ViewVolumeNode> const& cameraNode)
{
    mCameraNode = cameraNode;
}

void CameraAndLightNodesWindow3::CameraNodeRig::MoveForward()
{
    Vector4<float> position = mCameraNode->localTransform.GetTranslationW1();
    Matrix4x4<float> rotate = mCameraNode->localTransform.GetRotation();
    Vector4<float> direction = GetBasis(rotate, 0);
    position += mTranslationSpeed * direction;
    mCameraNode->localTransform.SetTranslation(position);
    mCameraNode->Update();
}

void CameraAndLightNodesWindow3::CameraNodeRig::MoveBackward()
{
    Vector4<float> position = mCameraNode->localTransform.GetTranslationW1();
    position -= mTranslationSpeed * mCamera->GetDVector();
    mCameraNode->localTransform.SetTranslation(position);
    mCameraNode->Update();
}

void CameraAndLightNodesWindow3::CameraNodeRig::TurnRight()
{
    Matrix4x4<float> rotate = mCameraNode->localTransform.GetRotation();
    Vector4<float> uVector = GetBasis(rotate, 1);
    AxisAngle<4, float> aa(uVector, -mRotationSpeed);
    Matrix4x4<float> increment = Rotation<4, float>(aa);
    mCameraNode->localTransform.SetRotation(DoTransform(increment, rotate));
    mCameraNode->Update();
}

void CameraAndLightNodesWindow3::CameraNodeRig::TurnLeft()
{
    Matrix4x4<float> rotate = mCameraNode->localTransform.GetRotation();
    Vector4<float> uVector = GetBasis(rotate, 1);
    AxisAngle<4, float> aa(uVector, +mRotationSpeed);
    Matrix4x4<float> increment = Rotation<4, float>(aa);
    mCameraNode->localTransform.SetRotation(DoTransform(increment, rotate));
    mCameraNode->Update();
}
