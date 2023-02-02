// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "ProjectedTexturesWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>

ProjectedTexturesWindow3::ProjectedTexturesWindow3(Parameters& parameters)
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
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.01f, 0.001f,
        { 0.0f, 0.0f, z }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void ProjectedTexturesWindow3::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mTorus);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool ProjectedTexturesWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Magician.png") == "")
    {
        LogError("Cannot find file Magician.png");
        return false;
    }

    return true;
}

void ProjectedTexturesWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();

    struct Vertex
    {
        Vector3<float> position, normal;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mTorus = mf.CreateTorus(32, 32, 40.0f, 20.0f);

    auto material = std::make_shared<Material>();
    material->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    material->ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    material->diffuse = { 0.99607f, 0.83920f, 0.67059f, 1.0f };
    material->specular = { 0.8f, 0.8f, 0.8f, 0.0f };

    auto lighting = std::make_shared<Lighting>();
    lighting->ambient = { 0.25f, 0.25f, 0.25f, 1.0f };
    lighting->diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
    lighting->specular = { 0.0f, 0.0f, 0.0f, 1.0f };
    lighting->attenuation = { 1.0f, 0.0f, 0.0f, 1.0f };

    auto geometry = std::make_shared<LightCameraGeometry>();
    mLightWorldDirection = { 0.0f, 0.0f, 1.0f, 0.0f };

    std::string path = mEnvironment.GetPath("Magician.png");
    auto texture = WICFileIO::Load(path, false);
    mPTEffect = std::make_shared<ProjectedTextureEffect>(mProgramFactory, mUpdater,
        material, lighting, geometry, texture, SamplerState::Filter::MIN_L_MAG_L_MIP_P,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);

    mProjector = std::make_shared<Camera>(true, mEngine->HasDepthRange01());

    mProjector->SetFrustum(1.0f, 10.0f, -0.4125f, 0.4125f, -0.55f, 0.55f);
    Vector4<float> prjDVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> prjUVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> prjRVector = Cross(prjDVector, prjUVector);
    Vector4<float> prjPosition{ 0.0f, 0.0f, -200.0f, 1.0f };
    mProjector->SetFrame(prjPosition, prjDVector, prjUVector, prjRVector);

#if defined(GTE_USE_MAT_VEC)
    Matrix4x4<float> postProjectionMatrix{
        0.5f, 0.0f, 0.0f, 0.5f,
        0.0f, 0.5f, 0.0f, 0.5f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
#else
    Matrix4x4<float> postProjectionMatrix{
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f
    };
#endif
    mProjector->SetPostProjectionMatrix(postProjectionMatrix);

    mTorus->SetEffect(mPTEffect);
    mTorus->UpdateModelBound();
    mPVWMatrices.Subscribe(mTorus->worldTransform, mPTEffect->GetPVWMatrixConstant());
    mScene->AttachChild(mTorus);

    mTrackBall.Attach(mScene);
    mScene->Update();
}

void ProjectedTexturesWindow3::UpdateConstants()
{
    Matrix4x4<float> wMatrix = mTorus->worldTransform.GetHMatrix();
    Matrix4x4<float> projPVMatrix = mProjector->GetProjectionViewMatrix();
    Matrix4x4<float> invWMatrix = mTorus->worldTransform.GetHInverse();
    Vector4<float> cameraWorldPosition = mCamera->GetPosition();
    auto const& geometry = mPTEffect->GetGeometry();
    Matrix4x4<float> projPVWMatrix = DoTransform(projPVMatrix, wMatrix);
    geometry->cameraModelPosition = DoTransform(invWMatrix, cameraWorldPosition);
    geometry->lightModelDirection = DoTransform(invWMatrix, mLightWorldDirection);
    mPTEffect->SetProjectorMatrix(projPVWMatrix);
    mPTEffect->UpdateProjectorMatrixConstant();
    mPTEffect->UpdateGeometryConstant();
    mPVWMatrices.Update();
}
