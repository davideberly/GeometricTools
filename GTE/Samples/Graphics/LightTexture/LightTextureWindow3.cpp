// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "LightTextureWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <random>

LightTextureWindow3::LightTextureWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mUseDirectional(true)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.525f, 0.741f, 0.831f, 1.0f });

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.005f, 0.002f,
        { 0.0f, -7.0f, 1.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
}

void LightTextureWindow3::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mTerrain);

    std::array<float, 4> textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    mEngine->Draw(8, mYSize - 24, textColor, (mUseDirectional ? "Directional" : "Point"));
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());

    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool LightTextureWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 's':
    case 'S':
        if (mUseDirectional)
        {
            mPVWMatrices.Unsubscribe(mTerrain->worldTransform);
            mTerrain->SetEffect(mPLTEffect);
            mPVWMatrices.Subscribe(mTerrain->worldTransform, mPLTEffect->GetPVWMatrixConstant());
            mPVWMatrices.Update();
        }
        else
        {
            mPVWMatrices.Unsubscribe(mTerrain->worldTransform);
            mTerrain->SetEffect(mDLTEffect);
            mPVWMatrices.Subscribe(mTerrain->worldTransform, mDLTEffect->GetPVWMatrixConstant());
            mPVWMatrices.Update();
        }
        mUseDirectional = !mUseDirectional;
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

bool LightTextureWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "BTHeightField.png",
        "BTStone.png"
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

void LightTextureWindow3::CreateScene()
{
    mTrackBall.Set(mXSize, mYSize, mCamera);

    // Create the visual effect.  The world up-direction is (0,0,1).  Choose
    // the light to point down.
    auto material = std::make_shared<Material>();
    material->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    material->ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    material->diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
    material->specular = { 1.0f, 1.0f, 1.0f, 75.0f };

    auto lighting = std::make_shared<Lighting>();
    lighting->ambient = mEngine->GetClearColor();
    lighting->attenuation = { 1.0f, 0.0f, 0.0f, 1.0f };

    auto geometry = std::make_shared<LightCameraGeometry>();
    mLightWorldPosition = { 0.0f, 0.0f, 8.0f, 1.0f };
    mLightWorldDirection = { 0.0f, 0.0f, -1.0f, 0.0f };

    std::string stoneFile = mEnvironment.GetPath("BTStone.png");
    auto stoneTexture = WICFileIO::Load(stoneFile, true);
    stoneTexture->AutogenerateMipmaps();

    mDLTEffect = std::make_shared<DirectionalLightTextureEffect>(mProgramFactory,
        mUpdater, material, lighting, geometry, stoneTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);

    mPLTEffect = std::make_shared<PointLightTextureEffect>(mProgramFactory,
        mUpdater, material, lighting, geometry, stoneTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);

    // Create the height field for terrain using heights from a gray-scale
    // bitmap image.
    struct Vertex
    {
        Vector3<float> position, normal;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    std::string heightFile = mEnvironment.GetPath("BTHeightField.png");
    auto heightTexture = WICFileIO::Load(heightFile, false);
    MeshFactory mf;  // Fills in Vertex.position, Vertex.tcoord.
    mf.SetVertexFormat(vformat);
    mTerrain = mf.CreateRectangle(heightTexture->GetWidth(), heightTexture->GetHeight(), 8.0f, 8.0f);
    mTrackBall.Attach(mTerrain);

    // The mesh factory creates a flat height field.  Use the height-field
    // image to generate the heights and use a random number generator to
    // perturb them, just to add some noise.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);
    std::shared_ptr<VertexBuffer> vbuffer = mTerrain->GetVertexBuffer();
    uint32_t numVertices = vbuffer->GetNumElements();
    auto* vertices = vbuffer->Get<Vertex>();
    uint8_t* heights = heightTexture->Get<uint8_t>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        float height = static_cast<float>(heights[4 * i]) / 255.0f;
        float perturb = 0.05f * rnd(mte);
        vertices[i].position[2] = 3.0f * height + perturb;
    }

    mTerrain->SetEffect(mDLTEffect);
    mTerrain->UpdateModelNormals();  // Fill in Vertex.normal.
    mPVWMatrices.Subscribe(mTerrain->worldTransform, mDLTEffect->GetPVWMatrixConstant());
}

void LightTextureWindow3::UpdateConstants()
{
    Matrix4x4<float> invWMatrix = mTerrain->worldTransform.GetHInverse();
    Vector4<float> cameraWorldPosition = mCamera->GetPosition();
    auto const& geometry = mDLTEffect->GetGeometry();
    geometry->cameraModelPosition = DoTransform(invWMatrix, cameraWorldPosition);
    if (mUseDirectional)
    {
        geometry->lightModelDirection = DoTransform(invWMatrix, mLightWorldDirection);
        mDLTEffect->UpdateGeometryConstant();
    }
    else
    {
        geometry->lightModelPosition = DoTransform(invWMatrix, mLightWorldPosition);
        mPLTEffect->UpdateGeometryConstant();
    }
    mPVWMatrices.Update();
}

