// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "PlaneMeshIntersectionWindow3.h"
#include <Graphics/MeshFactory.h>

PlaneMeshIntersectionWindow3::PlaneMeshIntersectionWindow3(
    Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    mPSTarget = std::make_shared<DrawTarget>(2, DF_R32G32B32A32_FLOAT,
        mXSize, mYSize, true, false, DF_D24_UNORM_S8_UINT, false);
    mPSColor = mPSTarget->GetRTTexture(0);
    mPSPlaneConstant = mPSTarget->GetRTTexture(1);

    mScreen = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, mXSize, mYSize);
    mScreen->SetUsage(Resource::Usage::SHADER_OUTPUT);
    mScreen->SetCopy(Resource::Copy::STAGING_TO_CPU);

    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize, mYSize, mXSize, mYSize,
        SamplerState::Filter::MIN_P_MAG_P_MIP_P, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP, true);
    mOverlay->SetTexture(mScreen);

    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, std::numeric_limits<float>::max() });

    auto const& cshader = mDrawIntersections->GetComputeShader();
    cshader->Set("colorImage", mPSColor);
    cshader->Set("planeConstantImage", mPSPlaneConstant);
    cshader->Set("outputImage", mScreen);

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        {0.0f, 0.0f, -2.5f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();
}

void PlaneMeshIntersectionWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }
    UpdateMatrices();

    mEngine->Enable(mPSTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Disable(mPSTarget);
    mEngine->Execute(mDrawIntersections, mXSize / 8, mYSize / 8, 1);
    mEngine->Draw(mOverlay);
    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 1.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool PlaneMeshIntersectionWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/PlaneMeshIntersection/Shaders/");
    std::vector<std::string> inputs =
    {
        mEngine->GetShaderName("PlaneMeshIntersection.vs"),
        mEngine->GetShaderName("PlaneMeshIntersection.ps"),
        mEngine->GetShaderName("DrawIntersections.cs")
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

bool PlaneMeshIntersectionWindow3::CreateScene()
{
    std::string vsPath = mEnvironment.GetPath(mEngine->GetShaderName("PlaneMeshIntersection.vs"));
    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("PlaneMeshIntersection.ps"));
    auto program = mProgramFactory->CreateFromFiles(vsPath, psPath, "");
    if (!program)
    {
        return false;
    }

    std::string csPath = mEnvironment.GetPath(mEngine->GetShaderName("DrawIntersections.cs"));
    mDrawIntersections = mProgramFactory->CreateFromFile(csPath);
    if (!mDrawIntersections)
    {
        return false;
    }

    float planeDelta = 0.125f;
    mPMIParameters = std::make_shared<ConstantBuffer>(sizeof(PMIParameters), true);
    PMIParameters& p = *mPMIParameters->Get<PMIParameters>();
    p.pvMatrix = mCamera->GetProjectionViewMatrix();
    p.wMatrix = Matrix4x4<float>::Identity();
    p.planeVector0 = Vector4<float>{ 1.0f, 0.0f, 0.0f, 0.0f } / planeDelta;
    p.planeVector1 = Vector4<float>{ 0.0f, 1.0f, 0.0f, 0.0f } / planeDelta;
    program->GetVertexShader()->Set("PMIParameters", mPMIParameters);

    auto effect = std::make_shared<VisualEffect>(program);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mMesh = mf.CreateSphere(16, 16, 1.0f);
    mMesh->SetEffect(effect);
    mMesh->Update();
    return true;
}

void PlaneMeshIntersectionWindow3::UpdateMatrices()
{
    PMIParameters& p = *mPMIParameters->Get<PMIParameters>();
    p.pvMatrix = mCamera->GetProjectionViewMatrix();
    Matrix4x4<float> wMatrix = mMesh->worldTransform;
    p.wMatrix = DoTransform(mTrackBall.GetOrientation(), wMatrix);
    mEngine->Update(mPMIParameters);
}
