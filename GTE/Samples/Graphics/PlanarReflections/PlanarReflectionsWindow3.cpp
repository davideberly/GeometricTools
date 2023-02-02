// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.03.06

#include "PlanarReflectionsWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>

PlanarReflectionsWindow3::PlanarReflectionsWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mFloor{},
    mWall{},
    mDodecahedron{},
    mTorus{},
    mReflectionCaster{},
    mPlanarReflectionEffect{}
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.001f, 0.01f,
        { 6.75f, 0.0f, 2.3f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();

    mPVWMatrices.Update();
}

void PlanarReflectionsWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mPlanarReflectionEffect->Draw(mEngine, mPVWMatrices);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool PlanarReflectionsWindow3::SetEnvironment()
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

void PlanarReflectionsWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();
    CreateFloor();
    CreateWall();
    CreateDodecahedron();
    CreateTorus();

    mReflectionCaster = std::make_shared<Node>();
    mReflectionCaster->AttachChild(mDodecahedron);
    mReflectionCaster->AttachChild(mTorus);

    mTrackBall.Attach(mScene);
    mScene->AttachChild(mFloor);
    mScene->AttachChild(mWall);
    mScene->AttachChild(mReflectionCaster);

    std::vector<std::shared_ptr<Visual>> planeVisuals = { mFloor, mWall };
    std::vector<float> reflectances = { 0.2f, 0.5f };
    mPlanarReflectionEffect = std::make_shared<PlanarReflectionEffect>(
        mReflectionCaster, planeVisuals, reflectances);

    mTrackBall.Update();
}

void PlanarReflectionsWindow3::CreateFloor()
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

void PlanarReflectionsWindow3::CreateWall()
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

void PlanarReflectionsWindow3::CreateDodecahedron()
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

void PlanarReflectionsWindow3::CreateTorus()
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
