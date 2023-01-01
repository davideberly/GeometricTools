// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BlendedTerrainWindow3.h"
#include <Applications/WICFileIO.h>
#include <random>

BlendedTerrainWindow3::BlendedTerrainWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mFlowDelta(0.00002f),
    mPowerDelta(1.125f),
    mZAngle(0.0f),
    mZDeltaAngle(0.00002f)
{
    if (!SetEnvironment() || !CreateTerrain())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    CreateSkyDome();
    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.005f, 0.002f,
        { 0.0f, -7.0f, 1.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
}

void BlendedTerrainWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }
    Update();

    mEngine->ClearBuffers();
    mEngine->Draw(mTerrain);
    mEngine->Draw(mSkyDome);
    mEngine->Draw(8, GetYSize() - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool BlendedTerrainWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
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

    case 'p':
    case 'P':
        mTerrainEffect->SetPowerFactor(mTerrainEffect->GetPowerFactor() * mPowerDelta);
        mEngine->Update(mTerrainEffect->GetPowerFactorConstant());
        return true;

    case 'm':
    case 'M':
        mTerrainEffect->SetPowerFactor(mTerrainEffect->GetPowerFactor() / mPowerDelta);
        mEngine->Update(mTerrainEffect->GetPowerFactorConstant());
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool BlendedTerrainWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/BlendedTerrain/Shaders/");
    mEnvironment.Insert(path + "/Samples/Data/");

    std::vector<std::string> inputs =
    {
        "BTHeightField.png",
        "BTGrass.png",
        "BTStone.png",
        "BTCloud.png",
        "SkyDome.png",
        "SkyDome.txt"
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

bool BlendedTerrainWindow3::CreateTerrain()
{
    // Load the height field for vertex displacement.
    std::string heightFile = mEnvironment.GetPath("BTHeightField.png");

    // Create the visual effect.
    bool created = false;
    mTerrainEffect = std::make_shared<BlendedTerrainEffect>(mEngine,
        mProgramFactory, mEnvironment, created);
    if (!created)
    {
        LogError("Failed to create the terrain effect.");
        return false;
    }

    // Create the vertex buffer for terrain.
    uint32_t const numSamples0 = 64, numSamples1 = 64;
    uint32_t const numVertices = numSamples0 * numSamples1;

    struct TerrainVertex
    {
        Vector3<float> position;
        Vector2<float> tcoord0;
        float tcoord1;
        Vector2<float> tcoord2;
    };
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32_FLOAT, 1);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 2);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);

    // Generate the geometry for a flat height field.
    TerrainVertex* vertex = vbuffer->Get<TerrainVertex>();
    float const extent0 = 8.0f, extent1 = 8.0f;
    float const inv0 = 1.0f / (static_cast<float>(numSamples0) - 1.0f);
    float const inv1 = 1.0f / (static_cast<float>(numSamples1) - 1.0f);
    Vector3<float> position{};
    Vector2<float> tcoord{};
    uint32_t i, i0, i1;
    for (i1 = 0, i = 0; i1 < numSamples1; ++i1)
    {
        tcoord[1] = i1 * inv1;
        position[1] = (2.0f * tcoord[1] - 1.0f) * extent1;
        for (i0 = 0; i0 < numSamples0; ++i0, ++i)
        {
            tcoord[0] = i0 * inv0;
            position[0] = (2.0f * tcoord[0] - 1.0f) * extent0;
            vertex[i].position = position;
            vertex[i].tcoord0 = tcoord;
            vertex[i].tcoord1 = 0.0f;
            vertex[i].tcoord2 = tcoord;
        }
    }

    // Use a Mersenne twister engine for random numbers.
    std::mt19937 mte;
    std::uniform_real_distribution<float> symrnd(-1.0f, 1.0f);

    // Set the heights based on a precomputed height field.  The image is
    // known to be 64x64, which matches numSamples0 and numSamples1.  It is
    // also gray scale, so we use only the red channel.
    auto texture = WICFileIO::Load(heightFile, false);
    uint8_t* image = texture->Get<uint8_t>();
    for (i = 0; i < numVertices; i++)
    {
        float height = static_cast<float>(image[4 * i]) / 255.0f;
        float perturb = 0.05f * symrnd(mte);
        vertex[i].position[2] = 3.0f * height + perturb;
        vertex[i].tcoord0 *= 8.0f;
        vertex[i].tcoord1 = height;
    }

    // Generate the index array for a regular grid of squares, each square a
    // pair of triangles.
    uint32_t const numTriangles = 2 * (numSamples0 - 1) * (numSamples1 - 1);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(uint32_t));
    uint32_t* indices = ibuffer->Get<uint32_t>();
    for (i1 = 0, i = 0; i1 < numSamples1 - 1; ++i1)
    {
        for (i0 = 0; i0 < numSamples0 - 1; ++i0)
        {
            int32_t v0 = i0 + numSamples0 * i1;
            int32_t v1 = v0 + 1;
            int32_t v2 = v1 + numSamples0;
            int32_t v3 = v0 + numSamples0;
            *indices++ = v0;
            *indices++ = v1;
            *indices++ = v2;
            *indices++ = v0;
            *indices++ = v2;
            *indices++ = v3;
        }
    }

    // Create the visual object.
    mTerrain = std::make_shared<Visual>(vbuffer, ibuffer, mTerrainEffect);
    mPVWMatrices.Subscribe(mTerrain->worldTransform, mTerrainEffect->GetPVWMatrixConstant());
    mTrackBall.Attach(mTerrain);
    return true;
}

void BlendedTerrainWindow3::CreateSkyDome()
{
    // Load the vertices and indices from file for the sky dome trimesh.
    std::string name = mEnvironment.GetPath("SkyDome.txt");
    std::ifstream inFile(name.c_str());

    uint32_t numVertices, numIndices, i;
    inFile >> numVertices;
    inFile >> numIndices;

    struct SkyDomeVertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    auto* vertices = vbuffer->Get<SkyDomeVertex>();
    for (i = 0; i < numVertices; ++i)
    {
        inFile >> vertices[i].position[0];
        inFile >> vertices[i].position[1];
        inFile >> vertices[i].position[2];
        inFile >> vertices[i].tcoord[0];
        inFile >> vertices[i].tcoord[1];
    }

    int32_t const numTriangles = numIndices / 3;
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(uint32_t));
    auto* indices = ibuffer->Get<int32_t>();
    for (i = 0; i < numIndices; ++i, ++indices)
    {
        inFile >> *indices;
    }

    inFile.close();

    // Load the sky texture.
    name = mEnvironment.GetPath("SkyDome.png");
    auto sky = WICFileIO::Load(name, true);
    sky->AutogenerateMipmaps();

    // Create the visual effect.
    mSkyDomeEffect = std::make_shared<Texture2Effect>(mProgramFactory, sky,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP, SamplerState::Mode::WRAP);

    // Create the visual object.
    mSkyDome = std::make_shared<Visual>(vbuffer, ibuffer, mSkyDomeEffect);

    // The sky dome needs to be translated and scaled for this sample.
    mSkyDome->localTransform.SetUniformScale(7.9f);
    mSkyDome->localTransform.SetTranslation(0.0f, 0.0f, -0.1f);
    mSkyDome->Update();
    mPVWMatrices.Subscribe(mSkyDome->worldTransform, mSkyDomeEffect->GetPVWMatrixConstant());
    mTrackBall.Attach(mSkyDome);
}

void BlendedTerrainWindow3::Update()
{
    // Animate the cloud layer.
    Vector2<float> flowDirection = mTerrainEffect->GetFlowDirection();
    flowDirection[0] += mFlowDelta;
    if (0.0f > flowDirection[0])
    {
        flowDirection[0] += 1.0f;
    }
    else if (1.0f < flowDirection[0])
    {
        flowDirection[0] -= 1.0f;
    }
    mTerrainEffect->SetFlowDirection(flowDirection);
    mEngine->Update(mTerrainEffect->GetFlowDirectionConstant());

    // Rotate the sky dome.
    mZAngle -= mZDeltaAngle;
    if (mZAngle < (float)-GTE_C_TWO_PI)
    {
        mZAngle += (float)GTE_C_TWO_PI;
    }
    mSkyDome->localTransform.SetRotation(
        AxisAngle<4, float>({ 0.0f, 0.0f, 1.0f, 0.0f }, -mZAngle));
    mSkyDome->Update();
    mPVWMatrices.Update();
}
