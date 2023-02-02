// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "LightsWindow3.h"
#include <Graphics/MeshFactory.h>
#include <Graphics/DirectionalLightEffect.h>
#include <Graphics/PointLightEffect.h>
#include <Graphics/SpotLightEffect.h>

LightsWindow3::LightsWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mEngine->SetClearColor({ 0.0f, 0.25f, 0.75f, 1.0f });
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    CreateScene();

    Vector3<float> pos{ 16.0f, 0.0f, 8.0f };
    Vector3<float> dir = -pos;
    Normalize(dir);
    Vector3<float> up{ dir[2], 0.0f, -dir[0] };
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { pos[0], pos[1], pos[2] }, { dir[0], dir[1], dir[2] }, { up[0], up[1], up[2] });
    mPVWMatrices.Update();
}

void LightsWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mPlane[0]);
    mEngine->Draw(mPlane[1]);
    mEngine->Draw(mSphere[0]);
    mEngine->Draw(mSphere[1]);
    std::array<float, 4> textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    mEngine->Draw(8, 16, textColor, mCaption[mType]);
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool LightsWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':   // toggle wireframe
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

    case 'd':   // use directional lights
    case 'D':
        UseLightType(LDIR);
        return true;

    case 'p':   // use point lights
    case 'P':
        UseLightType(LPNT);
        return true;

    case 's':   // use spot lights
    case 'S':
        UseLightType(LSPT);
        return true;

    // NOTE:  The lighting constant buffer is shared between vertex and pixel
    // shaders.  Therefore, the modification of a lighting member must occur
    // only once.  This is the case for cases 'i', 'I', 'a', 'A', 'e' and 'E'.

    case 'i':   // decrease light intensity
        for (int32_t lt = 0; lt < LNUM; ++lt)
        {
            for (int32_t gt = 0; gt < GNUM; ++gt)
            {
                auto effect0 = mEffect[lt][gt][0];
                auto lighting0 = effect0->GetLighting();
                lighting0->attenuation[3] -= 0.125f;
                lighting0->attenuation[3] = std::max(lighting0->attenuation[3], 0.0f);
                effect0->UpdateLightingConstant();
                auto effect1 = mEffect[lt][gt][1];
                auto lighting1 = effect1->GetLighting();
                lighting1->attenuation = lighting0->attenuation;
                effect1->UpdateLightingConstant();
            }
        }
        return true;

    case 'I':   // increase light intensity
        for (int32_t lt = 0; lt < LNUM; ++lt)
        {
            for (int32_t gt = 0; gt < GNUM; ++gt)
            {
                auto effect0 = mEffect[lt][gt][0];
                auto lighting0 = effect0->GetLighting();
                lighting0->attenuation[3] += 0.125f;
                lighting0->attenuation[3] = std::max(lighting0->attenuation[3], 0.0f);
                effect0->UpdateLightingConstant();
                auto effect1 = mEffect[lt][gt][1];
                auto lighting1 = effect1->GetLighting();
                lighting1->attenuation = lighting0->attenuation;
                effect1->UpdateLightingConstant();
            }
        }
        return true;

    case 'a':   // decrease spot angle
        for (int32_t gt = 0; gt < GNUM; ++gt)
        {
            auto effect0 = mEffect[LSPT][gt][0];
            auto lighting0 = effect0->GetLighting();
            lighting0->spotCutoff[0] -= 0.1f;
            lighting0->spotCutoff[0] = std::max(lighting0->spotCutoff[0], 0.0f);
            lighting0->spotCutoff[1] = std::cos(lighting0->spotCutoff[0]);
            lighting0->spotCutoff[2] = std::sin(lighting0->spotCutoff[0]);
            effect0->UpdateLightingConstant();
            auto effect1 = mEffect[LSPT][gt][1];
            auto lighting1 = effect0->GetLighting();
            lighting1->spotCutoff = lighting0->spotCutoff;
            effect1->UpdateLightingConstant();
        }
        return true;

    case 'A':   // increase spot angle
        for (int32_t gt = 0; gt < GNUM; ++gt)
        {
            auto effect0 = mEffect[LSPT][gt][0];
            auto lighting0 = effect0->GetLighting();
            lighting0->spotCutoff[0] += 0.1f;
            lighting0->spotCutoff[0] = std::max(lighting0->spotCutoff[0], 0.0f);
            lighting0->spotCutoff[1] = std::cos(lighting0->spotCutoff[0]);
            lighting0->spotCutoff[2] = std::sin(lighting0->spotCutoff[0]);
            effect0->UpdateLightingConstant();
            auto effect1 = mEffect[LSPT][gt][1];
            auto lighting1 = effect0->GetLighting();
            lighting1->spotCutoff = lighting0->spotCutoff;
            effect1->UpdateLightingConstant();
        }
        return true;

    case 'e':   // decrease spot exponent
        for (int32_t gt = 0; gt < GNUM; ++gt)
        {
            auto effect0 = mEffect[LSPT][gt][0];
            auto lighting0 = effect0->GetLighting();
            lighting0->spotCutoff[3] *= 0.5f;
            effect0->UpdateLightingConstant();
            auto effect1 = mEffect[LSPT][gt][1];
            auto lighting1 = effect1->GetLighting();
            lighting1->spotCutoff = lighting0->spotCutoff;
            effect1->UpdateLightingConstant();
        }
        return true;

    case 'E':   // increase spot exponent
        for (int32_t gt = 0; gt < GNUM; ++gt)
        {
            auto effect0 = mEffect[LSPT][gt][0];
            auto lighting0 = effect0->GetLighting();
            lighting0->spotCutoff[3] *= 2.0f;
            effect0->UpdateLightingConstant();
            auto effect1 = mEffect[LSPT][gt][1];
            auto lighting1 = effect1->GetLighting();
            lighting1->spotCutoff = lighting0->spotCutoff;
            effect1->UpdateLightingConstant();
        }
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

void LightsWindow3::CreateScene()
{
    // Copper color for the planes.
    Vector4<float> planeAmbient{ 0.2295f, 0.08825f, 0.0275f, 1.0f };
    Vector4<float> planeDiffuse{ 0.5508f, 0.2118f, 0.066f, 1.0f };
    Vector4<float> planeSpecular{ 0.580594f, 0.223257f, 0.0695701f, 51.2f };

    // Gold color for the spheres.
    Vector4<float> sphereAmbient{ 0.24725f, 0.2245f, 0.0645f, 1.0f };
    Vector4<float> sphereDiffuse{ 0.34615f, 0.3143f, 0.0903f, 1.0f };
    Vector4<float> sphereSpecular{ 0.797357f, 0.723991f, 0.208006f, 83.2f };

    // Various parameters shared by the lighting constants.  The geometric
    // parameters are dynamic, modified by UpdateConstants() whenever the
    // camera or scene moves.  These include camera model position, light
    // model position, light model direction, and model-to-world matrix.
    // The typecasts on cos/sin are to avoid an incorrect g++ warning with
    // Fedora 21 Linux.
    Vector4<float> darkGray{ 0.1f, 0.1f, 0.1f, 1.0f };
    Vector4<float> lightGray{ 0.75f, 0.75f, 0.75f, 1.0f };
    float angle = 0.125f * (float)GTE_C_PI;
    Vector4<float> lightSpotCutoff{ angle, std::cos(angle), std::sin(angle), 1.0f };

    mLightWorldPosition[SVTX] = { 4.0f, 4.0f - 8.0f, 8.0f, 1.0f };
    mLightWorldPosition[SPXL] = { 4.0f, 4.0f + 8.0f, 8.0f, 1.0f };
    mLightWorldDirection = { -1.0f, -1.0f, -1.0f, 0.0f };
    Normalize(mLightWorldDirection);

    std::shared_ptr<Material> material[LNUM][GNUM];
    std::shared_ptr<Lighting> lighting[LNUM][GNUM];
    std::shared_ptr<LightCameraGeometry> geometry[LNUM][GNUM];
    for (int32_t lt = 0; lt < LNUM; ++lt)
    {
        for (int32_t gt = 0; gt < GNUM; ++gt)
        {
            material[lt][gt] = std::make_shared<Material>();
            lighting[lt][gt] = std::make_shared<Lighting>();
            geometry[lt][gt] = std::make_shared<LightCameraGeometry>();
        }
    }

    // Initialize the directional lighting constants.
    material[LDIR][GPLN]->ambient = planeAmbient;
    material[LDIR][GPLN]->diffuse = planeDiffuse;
    material[LDIR][GPLN]->specular = planeSpecular;
    lighting[LDIR][GPLN]->ambient = lightGray;
    material[LDIR][GSPH]->ambient = sphereAmbient;
    material[LDIR][GSPH]->diffuse = sphereDiffuse;
    material[LDIR][GSPH]->specular = sphereSpecular;
    lighting[LDIR][GSPH]->ambient = lightGray;

    // Initialize the point lighting constants.
    material[LPNT][GPLN]->ambient = planeAmbient;
    material[LPNT][GPLN]->diffuse = planeDiffuse;
    material[LPNT][GPLN]->specular = planeSpecular;
    lighting[LPNT][GPLN]->ambient = darkGray;
    material[LPNT][GSPH]->ambient = sphereAmbient;
    material[LPNT][GSPH]->diffuse = sphereDiffuse;
    material[LPNT][GSPH]->specular = sphereSpecular;
    lighting[LPNT][GSPH]->ambient = darkGray;

    // Initialize the spot lighting constants.
    material[LSPT][GPLN]->ambient = planeAmbient;
    material[LSPT][GPLN]->diffuse = planeDiffuse;
    material[LSPT][GPLN]->specular = planeSpecular;
    lighting[LSPT][GPLN]->ambient = darkGray;
    lighting[LSPT][GPLN]->spotCutoff = lightSpotCutoff;
    material[LSPT][GSPH]->ambient = sphereAmbient;
    material[LSPT][GSPH]->diffuse = sphereDiffuse;
    material[LSPT][GSPH]->specular = sphereSpecular;
    lighting[LSPT][GSPH]->ambient = darkGray;
    lighting[LSPT][GSPH]->spotCutoff = lightSpotCutoff;

    // Create the effects.  Note that the material, lighting and geometry
    // constant buffers are shared by the vertex and pixel shaders.  This
    // is important to remember when processing keystroked; see the comments
    // in OnCharPress.
    for (int32_t gt = 0; gt < GNUM; ++gt)
    {
        for (int32_t st = 0; st < SNUM; ++st)
        {
            mEffect[LDIR][gt][st] = std::make_shared<DirectionalLightEffect>(
                mProgramFactory, mUpdater, st,
                material[LDIR][gt], lighting[LDIR][gt], geometry[LDIR][gt]);

            mEffect[LPNT][gt][st] = std::make_shared<PointLightEffect>(
                mProgramFactory, mUpdater, st,
                material[LPNT][gt], lighting[LPNT][gt], geometry[LPNT][gt]);

            mEffect[LSPT][gt][st] = std::make_shared<SpotLightEffect>(
                mProgramFactory, mUpdater, st,
                material[LSPT][gt], lighting[LSPT][gt], geometry[LSPT][gt]);
        }
    }

    // Create the planes and spheres.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mPlane[SVTX] = mf.CreateRectangle(128, 128, 8.0f, 8.0f);
    mPlane[SVTX]->localTransform.SetTranslation(0.0f, -8.0f, 0.0f);
    mTrackBall.Attach(mPlane[SVTX]);

    mPlane[SPXL] = mf.CreateRectangle(128, 128, 8.0f, 8.0f);
    mPlane[SPXL]->localTransform.SetTranslation(0.0f, +8.0f, 0.0f);
    mTrackBall.Attach(mPlane[SPXL]);

    mSphere[SVTX] = mf.CreateSphere(64, 64, 2.0f);
    mSphere[SVTX]->localTransform.SetTranslation(0.0f, -8.0f, 2.0f);
    mTrackBall.Attach(mSphere[SVTX]);

    mSphere[SPXL] = mf.CreateSphere(64, 64, 2.0f);
    mSphere[SPXL]->localTransform.SetTranslation(0.0f, +8.0f, 2.0f);
    mTrackBall.Attach(mSphere[SPXL]);

    mTrackBall.Update();

    mCaption[LDIR] = "Directional Light (left per vertex, right per pixel)";
    mCaption[LPNT] = "Point Light (left per vertex, right per pixel)";
    mCaption[LSPT] = "Spot Light (left per vertex, right per pixel)";

    UseLightType(LDIR);
}

void LightsWindow3::UseLightType(int32_t type)
{
    mPVWMatrices.Unsubscribe(mPlane[SVTX]->worldTransform);
    mPlane[SVTX]->SetEffect(mEffect[type][GPLN][SVTX]);
    mPVWMatrices.Subscribe(mPlane[SVTX]->worldTransform, mEffect[type][GPLN][SVTX]->GetPVWMatrixConstant());

    mPVWMatrices.Unsubscribe(mPlane[SPXL]->worldTransform);
    mPlane[SPXL]->SetEffect(mEffect[type][GPLN][SPXL]);
    mPVWMatrices.Subscribe(mPlane[SPXL]->worldTransform, mEffect[type][GPLN][SPXL]->GetPVWMatrixConstant());

    mPVWMatrices.Unsubscribe(mSphere[SVTX]->worldTransform);
    mSphere[SVTX]->SetEffect(mEffect[type][GSPH][SVTX]);
    mPVWMatrices.Subscribe(mSphere[SVTX]->worldTransform, mEffect[type][GSPH][SVTX]->GetPVWMatrixConstant());

    mPVWMatrices.Unsubscribe(mSphere[SPXL]->worldTransform);
    mSphere[SPXL]->SetEffect(mEffect[type][GSPH][SPXL]);
    mPVWMatrices.Subscribe(mSphere[SPXL]->worldTransform, mEffect[type][GSPH][SPXL]->GetPVWMatrixConstant());

    mType = type;

    mPVWMatrices.Update();
}

void LightsWindow3::UpdateConstants()
{
    // The pvw-matrices are updated automatically whenever the camera moves
    // or the trackball is rotated, which happens before this call.  Here we
    // need to update the camera model position, light model position, and
    // light model direction.

    // Compute the model-to-world transforms for the planes and spheres.
    Matrix4x4<float> wMatrix[GNUM][SNUM];
    Matrix4x4<float> rotate = mTrackBall.GetOrientation();
    wMatrix[GPLN][SVTX] = DoTransform(rotate, mPlane[SVTX]->worldTransform.GetHMatrix());
    wMatrix[GPLN][SPXL] = DoTransform(rotate, mPlane[SPXL]->worldTransform.GetHMatrix());
    wMatrix[GSPH][SVTX] = DoTransform(rotate, mSphere[SVTX]->worldTransform.GetHMatrix());
    wMatrix[GSPH][SPXL] = DoTransform(rotate, mSphere[SPXL]->worldTransform.GetHMatrix());

    // Compute the world-to-model transforms for the planes and spheres.
    Matrix4x4<float> invWMatrix[GNUM][SNUM];
    for (int32_t gt = 0; gt < GNUM; ++gt)
    {
        for (int32_t st = 0; st < SNUM; ++st)
        {
            invWMatrix[gt][st] = Inverse(wMatrix[gt][st]);
        }
    }

    Vector4<float> cameraWorldPosition = mCamera->GetPosition();
    for (int32_t lt = 0; lt < LNUM; ++lt)
    {
        for (int32_t gt = 0; gt < GNUM; ++gt)
        {
            for (int32_t st = 0; st < SNUM; ++st)
            {
                auto effect = mEffect[lt][gt][st];
                auto lighting = mEffect[lt][gt][st]->GetLighting();
                auto geometry = mEffect[lt][gt][st]->GetGeometry();
                auto const& invwmat = invWMatrix[gt][st];
                geometry->lightModelPosition = DoTransform(invwmat, mLightWorldPosition[st]);
                geometry->lightModelDirection = DoTransform(invwmat, mLightWorldDirection);
                geometry->cameraModelPosition = DoTransform(invwmat, cameraWorldPosition);
                effect->UpdateGeometryConstant();
            }
        }
    }
}
