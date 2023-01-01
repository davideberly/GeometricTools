// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

#include "ShadowMapsWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>

ShadowMapsWindow3::ShadowMapsWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mLightProjector(true, true),
    mShadowTargetSize(512),
    mShadowTarget{},
    mUnlitTarget{},
    mVisuals(2),
    mSceneEffects(2),
    mShadowEffects(2),
    mUnlitEffects(2),
    mBlurHEffect{},
    mBlurVEffect{},
    mBlurHTarget{},
    mBlurVTarget{}
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f });

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 8.0f, 0.0f, 4.0f }, { -0.894427180f, 0.0f,  -0.447213590f },
        { -0.447213590f, 0.0f, 0.894427180f });

    CreateLightProjector();
    CreateDrawTargets();
    CreateSceneEffects();
    CreateShadowEffects();
    CreateUnlitEffects();
    CreateBlurEffects();
    CreateScene();
}

void ShadowMapsWindow3::OnIdle()
{
    mTimer.Measure();
    (void)mCameraRig.Move();

    // Draw the scene from the light's perspective, writing the depths from
    // the light into the render target.
    DrawUsingShadowEffects();

    // Draw the scene from the camera's perspective using projected texturing
    // of the shadow map and determining which pixels are lit and which are
    // shadowed.
    DrawUsingUnlitEffects();

    // Gaussian blur (11x11) the unlit render target.
    ApplyBlur();

    // Draw the scene using regular textures, combining the shadow
    // information with the scene.
    DrawUsingSceneEffects();

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);
    mTimer.UpdateFrameCount();
}

bool ShadowMapsWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/Graphics/ShadowMaps/Shaders/");
    std::vector<std::string> inputs =
    {
        mEngine->GetShaderName("SMScene.vs"),
        mEngine->GetShaderName("SMScene.ps"),
        mEngine->GetShaderName("SMShadow.vs"),
        mEngine->GetShaderName("SMShadow.ps"),
        mEngine->GetShaderName("SMUnlit.vs"),
        mEngine->GetShaderName("SMUnlit.ps"),
        "Checkerboard.png",
        "Magician.png",
        "Stone.png"
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

void ShadowMapsWindow3::CreateLightProjector()
{
    Vector4<float> position{ 4.0f, 4.0f, 4.0f, 1.0f };
    Vector4<float> dVector{ -1.0f, -1.0f, -1.0f, 0.0f };
    Normalize(dVector);
    Vector4<float> uVector{ -1.0f, -1.0f, 2.0f, 0.0f };
    Normalize(uVector);
    Vector4<float> rVector = Cross(dVector, uVector);
    mLightProjector.SetFrustum(60.0f, 1.0f, 0.1f, 100.0f);
    mLightProjector.SetFrame(position, dVector, uVector, rVector);
}

void ShadowMapsWindow3::CreateDrawTargets()
{
    mShadowTarget = std::make_shared<DrawTarget>(1, DF_R32G32B32A32_FLOAT,
        mShadowTargetSize, mShadowTargetSize, false, true,
        DF_D24_UNORM_S8_UINT, false);
    mShadowTarget->GetRTTexture(0)->SetUsage(Resource::Usage::SHADER_OUTPUT);

    uint32_t width = static_cast<uint32_t>(mXSize);
    uint32_t height = static_cast<uint32_t>(mYSize);

    mUnlitTarget = std::make_shared<DrawTarget>(1, DF_R32G32B32A32_FLOAT,
        width, height, false, true, DF_D24_UNORM_S8_UINT, false);
    mUnlitTarget->GetRTTexture(0)->SetUsage(Resource::Usage::SHADER_OUTPUT);

    mBlurHTarget = std::make_shared<DrawTarget>(1, DF_R32G32B32A32_FLOAT,
        width, height);
    mBlurHTarget->GetRTTexture(0)->SetUsage(Resource::Usage::SHADER_OUTPUT);

    mBlurVTarget = std::make_shared<DrawTarget>(1, DF_R32G32B32A32_FLOAT,
        width, height);
    mBlurVTarget->GetRTTexture(0)->SetUsage(Resource::Usage::SHADER_OUTPUT);
}

void ShadowMapsWindow3::CreateSceneEffects()
{
    std::string vsName = mEngine->GetShaderName("SMScene.vs");
    std::string psName = mEngine->GetShaderName("SMScene.ps");
    std::string vsPath = mEnvironment.GetPath(vsName);
    std::string psPath = mEnvironment.GetPath(psName);

    SMSceneEffect::Geometry geometry{};
    geometry.worldMatrix = Matrix4x4<float>::Identity();
    geometry.lightPVMatrix = mLightProjector.GetProjectionViewMatrix();
    geometry.lightWorldPosition = mLightProjector.GetPosition();
    geometry.cameraWorldPosition = mCamera->GetPosition();

    SMSceneEffect::LightColor lightColor{};
    lightColor.color = { 1.0f, 1.0f, 1.0f, 1.0f };

    std::string stonePath = mEnvironment.GetPath("Stone.png");
    std::string checkerboardPath = mEnvironment.GetPath("Checkerboard.png");
    std::string magicianPath = mEnvironment.GetPath("Magician.png");
    auto stoneTexture = WICFileIO::Load(stonePath, false);
    auto checkerboardTexture = WICFileIO::Load(checkerboardPath, false);
    auto magicianTexture = WICFileIO::Load(magicianPath, false);

    mSceneEffects[0] = std::make_shared<SMSceneEffect>(mProgramFactory,
        vsPath, psPath, geometry, lightColor, stoneTexture,
        mBlurVTarget->GetRTTexture(0), magicianTexture);

    mSceneEffects[1] = std::make_shared<SMSceneEffect>(mProgramFactory,
        vsPath, psPath, geometry, lightColor, checkerboardTexture,
        mBlurVTarget->GetRTTexture(0), magicianTexture);
}

void ShadowMapsWindow3::CreateShadowEffects()
{
    std::string vsName = mEngine->GetShaderName("SMShadow.vs");
    std::string psName = mEngine->GetShaderName("SMShadow.ps");
    std::string vsPath = mEnvironment.GetPath(vsName);
    std::string psPath = mEnvironment.GetPath(psName);

    SMShadowEffect::Geometry geometry{};
    geometry.worldMatrix = Matrix4x4<float>::Zero();
    geometry.lightPVMatrix = mLightProjector.GetProjectionViewMatrix();

    mShadowEffects[0] = std::make_shared<SMShadowEffect>(mProgramFactory,
        vsPath, psPath, geometry);

    mShadowEffects[1] = std::make_shared<SMShadowEffect>(mProgramFactory,
        vsPath, psPath, geometry);
}

void ShadowMapsWindow3::CreateUnlitEffects()
{
    std::string vsName = mEngine->GetShaderName("SMUnlit.vs");
    std::string psName = mEngine->GetShaderName("SMUnlit.ps");
    std::string vsPath = mEnvironment.GetPath(vsName);
    std::string psPath = mEnvironment.GetPath(psName);

    SMUnlitEffect::Geometry geometry{};
    geometry.worldMatrix = Matrix4x4<float>::Zero();
    geometry.lightPVMatrix = mLightProjector.GetProjectionViewMatrix();

    SMUnlitEffect::Screen screen{};
    screen.value[0] = 0.1f;
    screen.value[1] = 1.0f / static_cast<float>(mShadowTargetSize);
    screen.value[2] = 1.0f / static_cast<float>(mShadowTargetSize);
    screen.value[3] = 0.0f;

    mUnlitEffects[0] = std::make_shared<SMUnlitEffect>(mProgramFactory,
        vsPath, psPath, geometry, screen, mShadowTarget->GetRTTexture(0));

    mUnlitEffects[1] = std::make_shared<SMUnlitEffect>(mProgramFactory,
        vsPath, psPath, geometry, screen, mShadowTarget->GetRTTexture(0));
}

void ShadowMapsWindow3::CreateBlurEffects()
{
    uint32_t const numXThreads = 8;
    uint32_t const numYThreads = 8;
    uint32_t const numXGroups = mBlurHTarget->GetWidth() / numXThreads;
    uint32_t const numYGroups = mBlurHTarget->GetHeight() / numYThreads;

    std::string csName = mEngine->GetShaderName("SMBlurH.cs");
    std::string csPath = mEnvironment.GetPath(csName);
    mBlurHEffect = std::make_shared<SMBlurEffect>(mProgramFactory,
        csPath, numXThreads, numYThreads, numXGroups, numYGroups);
    mBlurHEffect->SetInputImage(mUnlitTarget->GetRTTexture(0));
    mBlurHEffect->SetOutputImage(mBlurHTarget->GetRTTexture(0));

    csName = mEngine->GetShaderName("SMBlurV.cs");
    csPath = mEnvironment.GetPath(csName);
    mBlurVEffect = std::make_shared<SMBlurEffect>(mProgramFactory,
        csPath, numXThreads, numYThreads, numXGroups, numYGroups);
    mBlurVEffect->SetInputImage(mBlurHTarget->GetRTTexture(0));
    mBlurVEffect->SetOutputImage(mBlurVTarget->GetRTTexture(0));
}

void ShadowMapsWindow3::CreateScene()
{
    // Create a scene graph containing a plane and a sphere. The sphere will
    // cast a shadow on the plane. The plane is at index 0 and the sphere is
    // at index 1.
    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf(vformat);

    mVisuals[0] = mf.CreateRectangle(128, 128, 16.0f, 16.0f);
    mVisuals[0]->SetEffect(mSceneEffects[0]);
    mPVWMatrices.Subscribe(mVisuals[0]);
    mTrackBall.Attach(mVisuals[0]);

    mVisuals[1] = mf.CreateSphere(64, 64, 1.0f);
    mVisuals[1]->localTransform.SetTranslation(0.0f, 0.0f, 2.0f);
    mVisuals[1]->SetEffect(mSceneEffects[1]);
    mPVWMatrices.Subscribe(mVisuals[1]);
    mTrackBall.Attach(mVisuals[1]);

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void ShadowMapsWindow3::UpdateSceneEffects()
{
    SMSceneEffect::Geometry geometry{};
    geometry.lightPVMatrix = mLightProjector.GetProjectionViewMatrix();
    geometry.lightWorldPosition = mLightProjector.GetPosition();
    geometry.cameraWorldPosition = mCamera->GetPosition();

    for (size_t i = 0; i < mVisuals.size(); ++i)
    {
        auto const& visual = mVisuals[i];
        auto const& effect = mSceneEffects[i];
        auto const& gbuffer = effect->GetGeometryBuffer();
        geometry.worldMatrix = visual->worldTransform.GetHMatrix();
        *gbuffer->Get<SMSceneEffect::Geometry>() = geometry;
        mEngine->Update(gbuffer);
    }
}

void ShadowMapsWindow3::UpdateShadowEffects()
{
    SMShadowEffect::Geometry geometry{};
    geometry.lightPVMatrix = mLightProjector.GetProjectionViewMatrix();

    for (size_t i = 0; i < mVisuals.size(); ++i)
    {
        auto const& visual = mVisuals[i];
        auto const& effect = mShadowEffects[i];
        auto const& gbuffer = effect->GetGeometryBuffer();
        geometry.worldMatrix = visual->worldTransform.GetHMatrix();
        *gbuffer->Get<SMShadowEffect::Geometry>() = geometry;
        mEngine->Update(gbuffer);
    }
}

void ShadowMapsWindow3::UpdateUnlitEffects()
{
    SMUnlitEffect::Geometry geometry{};
    geometry.lightPVMatrix = mLightProjector.GetProjectionViewMatrix();

    for (size_t i = 0; i < mVisuals.size(); ++i)
    {
        auto const& visual = mVisuals[i];
        auto const& effect = mUnlitEffects[i];
        auto const& gbuffer = effect->GetGeometryBuffer();
        geometry.worldMatrix = visual->worldTransform.GetHMatrix();
        *gbuffer->Get<SMUnlitEffect::Geometry>() = geometry;
        mEngine->Update(gbuffer);
    }
}

void ShadowMapsWindow3::DrawUsingSceneEffects()
{
    // Remove the current effects.
    for (auto const& visual : mVisuals)
    {
        mPVWMatrices.Unsubscribe(visual);
    }

    // Restore the scene effects.
    for (size_t i = 0; i < mVisuals.size(); ++i)
    {
        auto const& visual = mVisuals[i];
        visual->SetEffect(mSceneEffects[i]);
        mPVWMatrices.Subscribe(visual);
    }
    mPVWMatrices.Update();
    UpdateSceneEffects();

    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f });
    mEngine->ClearBuffers();
    mEngine->Draw(mVisuals);
}

void ShadowMapsWindow3::DrawUsingShadowEffects()
{
    // Remove the current effects.
    for (auto const& visual : mVisuals)
    {
        mPVWMatrices.Unsubscribe(visual);
    }

    // Set the shadow effects.
    for (size_t i = 0; i < mVisuals.size(); ++i)
    {
        auto const& visual = mVisuals[i];
        visual->SetEffect(mShadowEffects[i]);
        mPVWMatrices.Subscribe(visual);
    }
    mPVWMatrices.Update();
    UpdateShadowEffects();

    // Draw the objects using the shadow effects.
    mEngine->Enable(mShadowTarget);
    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    mEngine->ClearBuffers();
    for (auto const& visual : mVisuals)
    {
        mEngine->Draw(visual);
    }
    mEngine->Disable(mShadowTarget);
}

void ShadowMapsWindow3::DrawUsingUnlitEffects()
{
    // Remove the current effects.
    for (auto const& visual : mVisuals)
    {
        mPVWMatrices.Unsubscribe(visual);
    }

    // Set the unlit effects.
    for (size_t i = 0; i < mVisuals.size(); ++i)
    {
        auto const& visual = mVisuals[i];
        visual->SetEffect(mUnlitEffects[i]);
        mPVWMatrices.Subscribe(visual);
    }
    mPVWMatrices.Update();
    UpdateUnlitEffects();

    // Draw the objects using the unlit effects.
    mEngine->Enable(mUnlitTarget);
    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    mEngine->ClearBuffers();
    for (auto const& visual : mVisuals)
    {
        mEngine->Draw(visual);
    }
    mEngine->Disable(mUnlitTarget);
}

void ShadowMapsWindow3::ApplyBlur()
{
    // Horizontally blur the unlit render target.
    mBlurHEffect->Execute(mEngine);

    // Vertically blur the horizontal blur render target.
    mBlurVEffect->Execute(mEngine);
}
