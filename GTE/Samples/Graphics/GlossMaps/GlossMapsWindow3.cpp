// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "GlossMapsWindow3.h"
#include <Applications/WICFileIO.h>

GlossMapsWindow3::GlossMapsWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    // Center the objects in the view frustum.
    CreateScene();
    mScene->localTransform.SetTranslation(-mScene->worldBound.GetCenter());
    float z = -2.0f * mScene->worldBound.GetRadius();
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 100.0f, 0.01f, 0.001f,
        { 0.0f, 0.0f, z }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void GlossMapsWindow3::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mSquareNoGloss);
    mEngine->Draw(mSquareGloss);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool GlossMapsWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Magic.png") == "")
    {
        LogError("Cannot find file Magic.png");
        return false;
    }

    return true;
}

void GlossMapsWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();

    struct Vertex
    {
        Vector3<float> position, normal;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    auto* vertices = vbuffer->Get<Vertex>();
    vertices[0].position = { -0.5f, 0.0f, -0.5f };
    vertices[0].normal = { 0.0f, 1.0f, 0.0f };
    vertices[0].tcoord = { 1.0f, 1.0f };
    vertices[1].position = { -0.5f, 0.0f, +0.5f };
    vertices[1].normal = { 0.0f, 1.0f, 0.0f };
    vertices[1].tcoord = { 1.0f, 0.0f };
    vertices[2].position = { +0.5f, 0.0f, +0.5f };
    vertices[2].normal = { 0.0f, 1.0f, 0.0f };
    vertices[2].tcoord = { 0.0f, 0.0f };
    vertices[3].position = { +0.5f, 0.0f, -0.5f };
    vertices[3].normal = { 0.0f, 1.0f, 0.0f };
    vertices[3].tcoord = { 0.0f, 1.0f };

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(uint32_t));
    auto* indices = ibuffer->Get<uint32_t>();
    indices[0] = 0;  indices[1] = 1;  indices[2] = 3;
    indices[3] = 3;  indices[4] = 1;  indices[5] = 2;

    auto material = std::make_shared<Material>();
    material->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    material->ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
    material->diffuse = { 0.7f, 0.7f, 0.7f, 1.0f };
    material->specular = { 1.0f, 1.0f, 1.0f, 25.0f };

    auto lighting = std::make_shared<Lighting>();
    lighting->ambient = { 0.1f, 0.1f, 0.1f, 1.0f };
    lighting->diffuse = { 0.6f, 0.6f, 0.6f, 1.0f };
    lighting->specular = { 1.0f, 1.0f, 1.0f, 1.0f };
    lighting->attenuation = { 1.0f, 0.0f, 0.0f, 1.0f };

    auto geometry0 = std::make_shared<LightCameraGeometry>();
    auto geometry1 = std::make_shared<LightCameraGeometry>();
    mLightWorldDirection = { 0.0f, -1.0f, 0.0f, 0.0f };

    AxisAngle<4, float> aa(Vector4<float>::Unit(0), static_cast<float>(-GTE_C_QUARTER_PI));

    // Create a non-gloss-mapped square.
    mDLEffect = std::make_shared<DirectionalLightEffect>(mProgramFactory, mUpdater, 0,
        material, lighting, geometry0);
    mSquareNoGloss = std::make_shared<Visual>(vbuffer, ibuffer, mDLEffect);
    mSquareNoGloss->localTransform.SetRotation(aa);
    mSquareNoGloss->localTransform.SetTranslation(1.0f, -1.0f, 0.0f);
    mSquareNoGloss->UpdateModelBound();
    mPVWMatrices.Subscribe(mSquareNoGloss->worldTransform, mDLEffect->GetPVWMatrixConstant());
    mScene->AttachChild(mSquareNoGloss);

    // Create a gloss-mapped square.
    std::string path = mEnvironment.GetPath("Magic.png");
    auto texture = WICFileIO::Load(path, false);
    mGMEffect = std::make_shared<GlossMapEffect>(mProgramFactory, mUpdater,
        material, lighting, geometry1, texture, SamplerState::Filter::MIN_L_MAG_L_MIP_P,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);
    mSquareGloss = std::make_shared<Visual>(vbuffer, ibuffer, mGMEffect);
    mSquareGloss->localTransform.SetRotation(aa);
    mSquareGloss->localTransform.SetTranslation(-1.0f, -1.0f, 0.0f);
    mSquareGloss->UpdateModelBound();
    mPVWMatrices.Subscribe(mSquareGloss->worldTransform, mGMEffect->GetPVWMatrixConstant());
    mScene->AttachChild(mSquareGloss);

    mTrackBall.Attach(mScene);
    mScene->Update();
}

void GlossMapsWindow3::UpdateConstants()
{
    Matrix4x4<float> invWMatrix0 = mSquareNoGloss->worldTransform.GetHInverse();
    Matrix4x4<float> invWMatrix1 = mSquareGloss->worldTransform.GetHInverse();
    Vector4<float> cameraWorldPosition = mCamera->GetPosition();
    auto const& geometry0 = mDLEffect->GetGeometry();
    auto const& geometry1 = mGMEffect->GetGeometry();
    geometry0->cameraModelPosition = DoTransform(invWMatrix0, cameraWorldPosition);
    geometry0->lightModelDirection = DoTransform(invWMatrix0, mLightWorldDirection);
    geometry1->cameraModelPosition = DoTransform(invWMatrix1, cameraWorldPosition);
    geometry1->lightModelDirection = DoTransform(invWMatrix1, mLightWorldDirection);
    mDLEffect->UpdateGeometryConstant();
    mGMEffect->UpdateGeometryConstant();
    mPVWMatrices.Update();
}
