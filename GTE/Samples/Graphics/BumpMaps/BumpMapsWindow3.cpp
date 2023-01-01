// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BumpMapsWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>

BumpMapsWindow3::BumpMapsWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mUseBumpMap(true)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 0.0f, -0.25f, -2.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    CreateScene();
}

void BumpMapsWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    if (mUseBumpMap)
    {
        mEngine->Draw(mBumpMappedTorus);
    }
    else
    {
        mEngine->Draw(mTexturedTorus);
    }

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool BumpMapsWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'b':
    case 'B':
    {
        mUseBumpMap = !mUseBumpMap;
        UpdateBumpMap();
        return true;
    }
    }

    return Window3::OnCharPress(key, x, y);
}

bool BumpMapsWindow3::OnMouseMotion(MouseButton button, int32_t x, int32_t y, uint32_t modifiers)
{
    if (Window3::OnMouseMotion(button, x, y, modifiers))
    {
        mPVWMatrices.Update();
        UpdateBumpMap();
    }
    return true;
}

bool BumpMapsWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "Bricks.png",
        "BricksNormal.png"
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

void BumpMapsWindow3::CreateScene()
{
    CreateBumpMappedTorus();
    CreateTexturedTorus();

    mScene = std::make_shared<Node>();
    mScene->AttachChild(mBumpMappedTorus);
    mScene->AttachChild(mTexturedTorus);
    mTrackBall.Attach(mScene);

    AxisAngle<4, float> aa(Vector4<float>::Unit(0), (float)GTE_C_QUARTER_PI);
    mBumpMappedTorus->localTransform.SetRotation(aa);
    mTexturedTorus->localTransform.SetRotation(aa);

    mTrackBall.Update();
    mPVWMatrices.Update();
    UpdateBumpMap();
}

void BumpMapsWindow3::CreateBumpMappedTorus()
{
    struct Vertex
    {
        Vector3<float> position;
        Vector3<float> normal;
        Vector3<float> lightDirection;
        Vector2<float> baseTCoord;
        Vector2<float> normalTCoord;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 1);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mf.SetVertexBufferUsage(Resource::Usage::DYNAMIC_UPDATE);
    mBumpMappedTorus = mf.CreateTorus(32, 32, 1.0f, 0.4f);
    auto const& vbuffer = mBumpMappedTorus->GetVertexBuffer();
    uint32_t const numVertices = vbuffer->GetNumElements();
    auto* vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].baseTCoord *= 4.0f;
        if (mUseBumpMap)
        {
            vertices[i].normalTCoord *= 4.0f;
        }
    }

    std::string texpath = mEnvironment.GetPath("Bricks.png");
    auto baseTexture = WICFileIO::Load(texpath, true);
    baseTexture->AutogenerateMipmaps();
    texpath = mEnvironment.GetPath("BricksNormal.png");
    auto normalTexture = WICFileIO::Load(texpath, true);
    normalTexture->AutogenerateMipmaps();
    mBumpMapEffect = std::make_shared<BumpMapEffect>(mProgramFactory,
        baseTexture, normalTexture, SamplerState::Filter::MIN_L_MAG_L_MIP_L,
        SamplerState::Mode::WRAP, SamplerState::Mode::WRAP);

    mBumpMappedTorus->SetEffect(mBumpMapEffect);
    mPVWMatrices.Subscribe(mBumpMappedTorus->worldTransform, mBumpMapEffect->GetPVWMatrixConstant());

    mLightDirection = Vector4<float>::Unit(2);
    BumpMapEffect::ComputeLightVectors(mBumpMappedTorus, mLightDirection);
}

void BumpMapsWindow3::CreateTexturedTorus()
{
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mf.SetVertexBufferUsage(Resource::Usage::DYNAMIC_UPDATE);
    mTexturedTorus = mf.CreateTorus(32, 32, 1.0f, 0.4f);
    auto const& vbuffer = mTexturedTorus->GetVertexBuffer();
    uint32_t const numVertices = vbuffer->GetNumElements();
    auto* vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].tcoord *= 4.0f;
    }

    std::string baseName = mEnvironment.GetPath("Bricks.png");
    std::shared_ptr<Texture2> baseTexture = WICFileIO::Load(baseName, true);
    baseTexture->AutogenerateMipmaps();

    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, baseTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP, SamplerState::Mode::WRAP);

    mTexturedTorus->SetEffect(effect);
    mPVWMatrices.Subscribe(mTexturedTorus->worldTransform, effect->GetPVWMatrixConstant());
}

void BumpMapsWindow3::UpdateBumpMap()
{
    if (mUseBumpMap)
    {
        // The scene graph transformations have been updated, which means the
        // tangent-space light vectors need updating.
        BumpMapEffect::ComputeLightVectors(mBumpMappedTorus, mLightDirection);
        mEngine->Update(mBumpMappedTorus->GetVertexBuffer());
    }
}
