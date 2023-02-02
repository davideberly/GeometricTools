// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "SphereMapsWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>

SphereMapsWindow3::SphereMapsWindow3(Parameters& parameters)
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
    float y = -2.0f * mScene->worldBound.GetRadius();
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.001f, 0.001f,
        { 0.0f, y, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void SphereMapsWindow3::OnIdle()
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

bool SphereMapsWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("SphereMap.png") == "")
    {
        LogError("Cannot find file SphereMap.png");
        return false;
    }

    return true;
}

void SphereMapsWindow3::CreateScene()
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
    mTorus = mf.CreateTorus(64, 64, 1.0f, 0.5f);

    std::string path = mEnvironment.GetPath("SphereMap.png");
    auto texture = WICFileIO::Load(path, false);
    mSMEffect = std::make_shared<SphereMapEffect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);

    mTorus->SetEffect(mSMEffect);
    mTorus->UpdateModelBound();
    mPVWMatrices.Subscribe(mTorus->worldTransform, mSMEffect->GetPVWMatrixConstant());
    mScene->AttachChild(mTorus);

    mTrackBall.Attach(mScene);
    mScene->Update();
}

void SphereMapsWindow3::UpdateConstants()
{
    Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
    Matrix4x4<float> vMatrix = mCamera->GetViewMatrix();
    Matrix4x4<float> wMatrix = mTorus->worldTransform.GetHMatrix();
    Matrix4x4<float> pvwMatrix = DoTransform(pvMatrix, wMatrix);
    Matrix4x4<float> vwMatrix = DoTransform(vMatrix, wMatrix);
    mSMEffect->SetPVWMatrix(pvwMatrix);
    mSMEffect->SetVWMatrix(vwMatrix);
    mEngine->Update(mSMEffect->GetPVWMatrixConstant());
    mEngine->Update(mSMEffect->GetVWMatrixConstant());
    mPVWMatrices.Update();
}
