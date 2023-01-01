// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.03.06

#include "PlanarShadowsWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>

PlanarShadowsWindow3::PlanarShadowsWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mScene{},
    mFloor{},
    mWall{},
    mDodecahedron{},
    mTorus{},
    mShadowCaster{},
    mLightProjector{},
    mPlanarShadowEffect{},
    mLPPosition{ 0.0f, 0.0f, 0.0f, 0.0f },
    mLPDirection{ 0.0f, 0.0f, 0.0f, 0.0f }
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mLPPosition = { 64.0f, 32.0f, 16.0f, 1.0f };
    mLPDirection = { -4.0f, -2.0f, -1.0f, 0.0f };
    Normalize(mLPDirection);

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.001f, 0.01f,
        { 6.75f, 0.0f, 2.3f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    // The camera parameters must be set before calling CreateScene()
    // because mLightProjector needs them.
    CreateScene();

    mPVWMatrices.Update();
}

void PlanarShadowsWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    // Maintain the light projector position and direction to be relative
    // to the scene. This ensures the shadow remains the same, which means
    // the virtual trackball just gives you the same shadow-cast scene from
    // different camera view points.
    auto const& hMatrix = mTrackBall.GetRoot()->worldTransform.GetHMatrix();
    mLightProjector->position = DoTransform(hMatrix, mLPPosition);
    mLightProjector->direction = DoTransform(hMatrix, mLPDirection);

    mEngine->ClearBuffers();
    mPlanarShadowEffect->Draw(mEngine, mPVWMatrices);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->Draw(8, 24, { 0.0f, 0.0f, 0.0f, 1.0f },
        (mLightProjector->isPointLight ? "point light" : "directional light"));
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool PlanarShadowsWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case ' ':
        mLightProjector->isPointLight = !mLightProjector->isPointLight;
        return true;
    };
    return Window3::OnCharPress(key, x, y);
}

bool PlanarShadowsWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "BallTexture.png",
        "Gravel.png",
        "Floor.png",
        "Wall1.png"
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

void PlanarShadowsWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();
    CreateFloor();
    CreateWall();
    CreateDodecahedron();
    CreateTorus();

    mShadowCaster = std::make_shared<Node>();
    mShadowCaster->AttachChild(mDodecahedron);
    mShadowCaster->AttachChild(mTorus);

    mTrackBall.Attach(mScene);
    mScene->AttachChild(mFloor);
    mScene->AttachChild(mWall);
    mScene->AttachChild(mShadowCaster);

    mLightProjector = std::make_shared<PlanarShadowEffect::LightProjector>();
    mLightProjector->isPointLight = false;

    Vector4<float> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    std::vector<Vector4<float>> shadowColors =
    {
        Vector4<float>{ 1.0f, 0.0f, 0.0f, 0.25f },
        Vector4<float>{ 0.0f, 1.0f, 0.0f, 0.25f }
    };
    std::vector<std::shared_ptr<Visual>> planeVisuals = { mFloor, mWall };
    mPlanarShadowEffect = std::make_shared<PlanarShadowEffect>(
        mProgramFactory, mShadowCaster, mLightProjector, planeVisuals,
        shadowColors);

    mTrackBall.Update();
}

void PlanarShadowsWindow3::CreateFloor()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    auto* vertices = vbuffer->Get<Vertex>();
    float const xExtent = 8.0f, yExtent = 16.0f, zValue = 0.0f;
    vertices[0].position = { -xExtent, -yExtent, zValue };
    vertices[1].position = { +xExtent, -yExtent, zValue };
    vertices[2].position = { +xExtent, +yExtent, zValue };
    vertices[3].position = { -xExtent, +yExtent, zValue };
    vertices[0].tcoord = { 0.0f, 0.0f };
    vertices[1].tcoord = { 1.0f, 0.0f };
    vertices[2].tcoord = { 1.0f, 1.0f };
    vertices[3].tcoord = { 0.0f, 1.0f };

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(uint32_t));
    auto indices = ibuffer->Get<uint32_t>();
    indices[0] = 0;  indices[1] = 1;  indices[2] = 2;
    indices[3] = 0;  indices[4] = 2;  indices[5] = 3;

    std::string path = mEnvironment.GetPath("Floor.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mFloor = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mFloor);
}

void PlanarShadowsWindow3::CreateWall()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    auto* vertices = vbuffer->Get<Vertex>();
    float const xValue = -8.0f, yExtent = 16.0f, zExtent = 16.0f, maxTCoord = 4.0f;

    vertices[0].position = { xValue, -yExtent, 0.0f };
    vertices[1].position = { xValue, +yExtent, 0.0f };
    vertices[2].position = { xValue, +yExtent, zExtent };
    vertices[3].position = { xValue, -yExtent, zExtent };
    vertices[0].tcoord = { 0.0f, 0.0f };
    vertices[1].tcoord = { maxTCoord, 0.0f };
    vertices[2].tcoord = { maxTCoord, maxTCoord };
    vertices[3].tcoord = { 0.0f, maxTCoord };

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(uint32_t));
    auto indices = ibuffer->Get<uint32_t>();
    indices[0] = 0;  indices[1] = 1;  indices[2] = 2;
    indices[3] = 0;  indices[4] = 2;  indices[5] = 3;

    std::string path = mEnvironment.GetPath("Wall1.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mWall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mWall);
}

void PlanarShadowsWindow3::CreateDodecahedron()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf{};
    mf.SetVertexFormat(vformat);
    mDodecahedron = mf.CreateDodecahedron();
    mDodecahedron->localTransform.SetTranslation(0.0f, 0.0f, 2.0f);

    std::string path = mEnvironment.GetPath("BallTexture.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mDodecahedron->SetEffect(effect);
    mPVWMatrices.Subscribe(mDodecahedron);
}

void PlanarShadowsWindow3::CreateTorus()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf{};
    mf.SetVertexFormat(vformat);
    mTorus = mf.CreateTorus(32, 32, 2.0f, 0.25f);
    mTorus->localTransform.SetTranslation(0.0f, 0.0f, 2.0f);
    AxisAngle<3, float> aa{};
    aa.axis = { 1.0f, 0.0f, 0.0f };
    aa.angle = static_cast<float>(-GTE_C_PI / 4.0);
    mTorus->localTransform.SetRotation(aa);

    std::string path = mEnvironment.GetPath("Gravel.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mTorus->SetEffect(effect);
    mPVWMatrices.Subscribe(mTorus);
}
