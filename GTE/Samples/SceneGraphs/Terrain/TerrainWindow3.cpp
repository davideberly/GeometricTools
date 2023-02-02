// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "TerrainWindow3.h"
#include "TerrainEffect.h"
#include <Applications/WICFileIO.h>
#include <Graphics/Texture2Effect.h>

TerrainWindow3::TerrainWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mLastUpdateTime(mMotionTimer.GetSeconds())
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mTextColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    mEngine->SetClearColor({ 0.5f, 0.0f, 1.0f, 1.0f });

    float const heightAboveTerrain = 20.0f;
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 1.0f, 1500.0f);
    Vector4<float> camPosition{ 64.0f, 64.0f, heightAboveTerrain, 1.0f };
    Vector4<float> camDVector{ (float)GTE_C_INV_SQRT_2, (float)GTE_C_INV_SQRT_2, 0.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    CreateScene();

    // Initialize the rig that keeps the camera heightAboveTerrain units
    // above the terrain.  The camera position is previously initialized to
    // the delta height and the OnKeyDown-Move-OnKeyUp calls add this to the
    // height of the terrain at the initial (x,y) = (64,64).
    mTerrainCameraRig.Initialize(mCamera, 5.0f, 0.01f, mTerrain, heightAboveTerrain);
    OnKeyDown(KEY_UP, 0, 0);
    mTerrainCameraRig.Move();
    OnKeyUp(KEY_UP, 0, 0);

    UpdateScene();
}

void TerrainWindow3::OnIdle()
{
    double time = mMotionTimer.GetSeconds();
    if (60.0 * (time - mLastUpdateTime) >= 1.0)
    {
        mLastUpdateTime = time;

        mTimer.Measure();

        if (mTerrainCameraRig.Move())
        {
            UpdateScene();
        }

        // Get the terrain height and normal vector and report it to the user.
        Vector4<float> camPosition = mCamera->GetPosition();
        float height = mTerrain->GetHeight(camPosition[0], camPosition[1]);
        Vector3<float> normal = mTerrain->GetNormal(camPosition[0], camPosition[1]);
        std::string message = "height = " + std::to_string(height) +
            " , normal = (" + std::to_string(normal[0]) + ", " +
            std::to_string(normal[1]) + ", " + std::to_string(normal[2]) + ")";

        mEngine->ClearBuffers();
        for (auto const visual : mCuller.GetVisibleSet())
        {
            mEngine->Draw(visual);
        }
        mEngine->Draw(8, mYSize - 8, mTextColor, mTimer.GetFPS());
        mEngine->Draw(128, mYSize - 8, mTextColor, message);
        mEngine->DisplayColorBuffer(0);

        mTimer.UpdateFrameCount();
    }
}

bool TerrainWindow3::OnKeyDown(int32_t key, int32_t, int32_t)
{
    return mTerrainCameraRig.PushMotion(key);
}

bool TerrainWindow3::OnKeyUp(int32_t key, int32_t, int32_t)
{
    return mTerrainCameraRig.PopMotion(key);
}

bool TerrainWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/SceneGraphs/Terrain/Data/");
    mEnvironment.Insert(path + "/Samples/SceneGraphs/Terrain/Shaders/");

    std::vector<std::string> inputs =
    {
        mEngine->GetShaderName("BaseMulDetailFogExpSqr.vs"),
        mEngine->GetShaderName("BaseMulDetailFogExpSqr.ps"),
        "SkyDome.txt",
        "SkyDome.png",
        "Detail.png",
        "height.information.txt"
    };

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    for (int32_t row = 0; row < 8; ++row)
    {
        for (int32_t col = 0; col < 8; ++col)
        {
            std::string suffix = "." + std::to_string(row) + "." + std::to_string(col);
            std::string name = "color" + suffix + ".png";
            if (mEnvironment.GetPath(name) == "")
            {
                LogError("Cannot find file " + name);
                return false;
            }
            name = "height" + suffix + ".binary";
            if (mEnvironment.GetPath(name) == "")
            {
                LogError("Cannot find file " + name);
                return false;
            }
        }
    }

    return true;
}

void TerrainWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();
    CreateTerrain();
    CreateSkyDome();
}

void TerrainWindow3::CreateTerrain()
{
    // Load the height field and create the terrain.
    std::string path = mEnvironment.GetPath("height.information.txt");
    std::ifstream input(path);
    size_t numRows, numCols, size;
    float minElevation, maxElevation, spacing;
    input >> numRows;       // 8
    input >> numCols;       // 8
    input >> size;          // 129
    input >> minElevation;  // 0.0
    input >> maxElevation;  // 200.0
    input >> spacing;       // 4.0
    input.close();

    // Create the terrain.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 1);
    mTerrain = std::make_shared<Terrain>(numRows, numCols, size, minElevation,
        maxElevation, spacing, vformat, mCamera);
    mScene->AttachChild(mTerrain);

    // Load the terrain page heights.
    std::vector<uint16_t> heights(size * size);
    for (size_t row = 0; row < numRows; ++row)
    {
        for (size_t col = 0; col < numCols; ++col)
        {
            std::string suffix = "." + std::to_string(row) + "." + std::to_string(col);
            path = mEnvironment.GetPath("height" + suffix + ".binary");
            input.open(path, std::ios::binary);
            input.read((char*)heights.data(), heights.size() * sizeof(uint16_t));
            input.close();

            mTerrain->SetHeights(row, col, heights);
        }
    }

    // Create the resources shared by the terrain-page effects.
    path = mEnvironment.GetPath("Detail.png");
    auto detailTexture = WICFileIO::Load(path, true);
    detailTexture->AutogenerateMipmaps();
    mEngine->Bind(detailTexture);

    Vector4<float> fogColorDensity{ 0.5686f, 0.7255f, 0.8353f, 0.0015f };

    std::string vsPath = mEnvironment.GetPath(mEngine->GetShaderName("BaseMulDetailFogExpSqr.vs"));
    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("BaseMulDetailFogExpSqr.ps"));

    // Attach an effect to each terrain page.  Preload all resouces to video
    // memory to avoid frame-rate stalls when new terrain pages are
    // encountered as the camera moves.
    std::string pagePrefix = "color.";
    std::string pageSuffix = ".png";
    for (size_t r = 0; r < numRows; ++r)
    {
        std::string rowID = std::to_string(r);
        for (size_t c = 0; c < numCols; ++c)
        {
            std::string colID = std::to_string(c);
            std::string pageID = rowID + "." + colID;
            path = mEnvironment.GetPath(pagePrefix + pageID + pageSuffix);
            auto colorTexture = WICFileIO::Load(path, true);
            colorTexture->AutogenerateMipmaps();
            mEngine->Bind(colorTexture);

            auto program = mProgramFactory->CreateFromFiles(vsPath, psPath, "");

            auto terrainEffect = std::make_shared<TerrainEffect>(program, colorTexture,
                detailTexture, fogColorDensity);

            auto page = mTerrain->GetPage(r, c);
            page->name = "page" + pageID;
            page->SetEffect(terrainEffect);
            mPVWMatrices.Subscribe(page->worldTransform, terrainEffect->GetPVWMatrixConstant());
            mEngine->Bind(page->GetVertexBuffer());
            mEngine->Bind(page->GetIndexBuffer());
        }
    }
}

void TerrainWindow3::CreateSkyDome()
{
    // Load the vertices and indices from file for the sky dome trimesh.
    std::string path = mEnvironment.GetPath("SkyDome.txt");
    std::ifstream inFile(path);

    uint32_t numVertices, numIndices, i;
    inFile >> numVertices;
    inFile >> numIndices;

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    SkyDomeVertex* vertex = vbuffer->Get<SkyDomeVertex>();
    for (i = 0; i < numVertices; ++i)
    {
        inFile >> vertex[i].position[0];
        inFile >> vertex[i].position[1];
        inFile >> vertex[i].position[2];
        inFile >> vertex[i].tcoord[0];
        inFile >> vertex[i].tcoord[1];
    }

    int32_t const numTriangles = numIndices / 3;
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(uint32_t));
    auto* indices = ibuffer->Get<uint32_t>();
    for (i = 0; i < numIndices; ++i, ++indices)
    {
        inFile >> *indices;
    }

    inFile.close();

    // Load the sky texture and create the texture effect for it.
    path = mEnvironment.GetPath("SkyDome.png");
    auto skyTexture = WICFileIO::Load(path, true);
    skyTexture->AutogenerateMipmaps();
    auto skyEffect = std::make_shared<Texture2Effect>(mProgramFactory, skyTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    // Create the sky dome object, positioning and scaling it to be centered
    // on the terrain and large enough to encompass the terrain.
    mSkyDome = std::make_shared<Visual>(vbuffer, ibuffer, skyEffect);
    Vector4<float> skyPosition = mCamera->GetPosition();
    skyPosition[2] = 0.0f;
    mSkyDome->localTransform.SetTranslation(skyPosition);
    mSkyDome->localTransform.SetUniformScale(mCamera->GetDMax());
    mSkyDome->UpdateModelBound();
    mSkyDome->UpdateModelNormals();
    mPVWMatrices.Subscribe(mSkyDome->worldTransform, skyEffect->GetPVWMatrixConstant());
    mScene->AttachChild(mSkyDome);
}

void TerrainWindow3::UpdateScene()
{
    // The sky dome moves with the camera so that it is always in view.
    Vector4<float> camPosition = mCamera->GetPosition();
    mSkyDome->localTransform.SetTranslation(camPosition[0], camPosition[1], 0.0f);
    mSkyDome->Update();

    // Update the active terrain pages, including the PVW and VW matrices.
    mTerrain->OnCameraMotion();
    mScene->Update();
    mCuller.ComputeVisibleSet(mCamera, mScene);
    mPVWMatrices.Update(mCuller.GetVisibleSet());
    for (auto const visual : mCuller.GetVisibleSet())
    {
        if (visual->name.find("page") == 0)
        {
            auto effect = std::dynamic_pointer_cast<TerrainEffect>(visual->GetEffect());
            auto const& vwMatrixConstant = effect->GetVWMatrixConstant();
            Matrix4x4<float> vMatrix = mCamera->GetViewMatrix();
            Matrix4x4<float> wMatrix = visual->worldTransform.GetHMatrix();
            *vwMatrixConstant->Get<Matrix4x4<float>>() = DoTransform(vMatrix, wMatrix);
            mEngine->Update(vwMatrixConstant);
        }
    }
}

void TerrainWindow3::TerrainCameraRig::Initialize(std::shared_ptr<Camera> const& camera,
    float translationSpeed, float rotationSpeed, std::shared_ptr<Terrain> const& terrain,
    float heightAboveTerrain)
{
    mCamera = camera;
    mTerrain = terrain;
    mHeightAboveTerrain = heightAboveTerrain;
    Set(mCamera, translationSpeed, rotationSpeed);
    RegisterMoveForward(KEY_UP);
    RegisterMoveBackward(KEY_DOWN);
    RegisterMoveUp(KEY_HOME);
    RegisterMoveDown(KEY_END);
    RegisterTurnRight(KEY_RIGHT);
    RegisterTurnLeft(KEY_LEFT);
}

void TerrainWindow3::TerrainCameraRig::MoveForward()
{
    CameraRig::MoveForward();
    Vector4<float> camPosition = mCamera->GetPosition();
    float height = mTerrain->GetHeight(camPosition[0], camPosition[1]);
    camPosition[2] = height + mHeightAboveTerrain;
    mCamera->SetPosition(camPosition);
}

void TerrainWindow3::TerrainCameraRig::MoveBackward()
{
    CameraRig::MoveBackward();
    Vector4<float> camPosition = mCamera->GetPosition();
    float height = mTerrain->GetHeight(camPosition[0], camPosition[1]);
    camPosition[2] = height + mHeightAboveTerrain;
    mCamera->SetPosition(camPosition);
}

void TerrainWindow3::TerrainCameraRig::MoveDown()
{
    if (mHeightAboveTerrain >= GetTranslationSpeed())
    {
        mHeightAboveTerrain -= GetTranslationSpeed();
    }

    Vector4<float> camPosition = mCamera->GetPosition();
    float height = mTerrain->GetHeight(camPosition[0], camPosition[1]);
    camPosition[2] = height + mHeightAboveTerrain;
    mCamera->SetPosition(camPosition);
}

void TerrainWindow3::TerrainCameraRig::MoveUp()
{
    mHeightAboveTerrain += GetTranslationSpeed();
    Vector4<float> camPosition = mCamera->GetPosition();
    float height = mTerrain->GetHeight(camPosition[0], camPosition[1]);
    camPosition[2] = height + mHeightAboveTerrain;
    mCamera->SetPosition(camPosition);
}
