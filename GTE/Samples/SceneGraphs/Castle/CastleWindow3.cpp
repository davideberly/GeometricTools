// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "CastleWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/ViewVolumeNode.h>

CastleWindow3::CastleWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    // Graphics engine state.
    mEngine->SetClearColor({ 0.6f, 0.851f, 0.918f, 1.0f});

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    // Create the scene and camera rig.
    CreateScene();
    InitializeFixedHeightRig();
    mFixedHeightRig.SetPicker(mScene, mPicker);

    // The model bounds must be updated first before the scene update.  The
    // latter update uses the model bounds to compute world bounds.
    UpdateVisualModelSpace(mScene.get());
    mScene->Update();

    OnKeyDown(KEY_UP, -1, -1);
    mFixedHeightRig.Move();
    OnKeyUp(KEY_UP, -1, -1);
    mPVWMatrices.Update();
    mCuller.ComputeVisibleSet(mCamera, mScene);
}

void CastleWindow3::OnIdle()
{
    mTimer.Measure();

    if (mFixedHeightRig.Move())
    {
        UpdateCameraLightModelPositions(mScene.get());
        mPVWMatrices.Update();
        mCuller.ComputeVisibleSet(mCamera, mScene);
    }

    mEngine->ClearBuffers();
    for (auto const& visual : mCuller.GetVisibleSet())
    {
        if (visual->name.find("Water") == std::string::npos)
        {
            mEngine->Draw(visual);
        }
    }

    mEngine->SetBlendState(mBlendState);
    mEngine->Draw(mWaterMesh[0]);
    mEngine->Draw(mWaterMesh[1]);
    mEngine->SetDefaultBlendState();

    std::array<float, 4> textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->Draw(8, 16, textColor, mPickMessage);
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool CastleWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() != mWireState)
        {
            mEngine->SetRasterizerState(mWireState);
        }
        else
        {
            mEngine->SetDefaultRasterizerState();
        }
        return true;
    case '+':
    case '=':
        mFixedHeightRig.SetVerticalDistance(mFixedHeightRig.GetVerticalDistance() + 0.1f);
        mFixedHeightRig.AdjustVerticalDistance();
        mPVWMatrices.Update();
        return true;
    case '-':
    case '_':
        mFixedHeightRig.SetVerticalDistance(mFixedHeightRig.GetVerticalDistance() - 0.1f);
        mFixedHeightRig.AdjustVerticalDistance();
        mPVWMatrices.Update();
        return true;
    case 't':  // Slower camera translation.
        mFixedHeightRig.SetTranslationSpeed(0.5f * mFixedHeightRig.GetTranslationSpeed());
        return true;

    case 'T':  // Faster camera translation.
        mFixedHeightRig.SetTranslationSpeed(2.0f * mFixedHeightRig.GetTranslationSpeed());
        return true;

    case 'r':  // Slower camera rotation.
        mFixedHeightRig.SetRotationSpeed(0.5f * mFixedHeightRig.GetRotationSpeed());
        return true;

    case 'R':  // Faster camera rotation.
        mFixedHeightRig.SetRotationSpeed(2.0f * mFixedHeightRig.GetRotationSpeed());
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool CastleWindow3::OnKeyDown(int32_t key, int32_t, int32_t)
{
    return mFixedHeightRig.PushMotion(key);
}

bool CastleWindow3::OnKeyUp(int32_t key, int32_t, int32_t)
{
    return mFixedHeightRig.PopMotion(key);
}

bool CastleWindow3::OnMouseClick(MouseButton button, MouseState state, int32_t x, int32_t y, uint32_t)
{
    if (button == MOUSE_LEFT && state == MOUSE_DOWN)
    {
        // Do a picking operation.  Reflect y to obtain right-handed window
        // coordinates.
        y = mYSize - 1 - y;
        int32_t viewX, viewY, viewW, viewH;
        mEngine->GetViewport(viewX, viewY, viewW, viewH);
        Vector4<float> origin, direction;
        if (mCamera->GetPickLine(viewX, viewY, viewW, viewH, x, y, origin, direction))
        {
            // Use a ray for picking.
            float tmin = 0.0f;
            float constexpr tmax = std::numeric_limits<float>::max();

            // We care only about intersecting a mesh, so request model-space
            // coordinates to avoid computing world-space information.
            mPicker(mScene, origin, direction, tmin, tmax);
            if (mPicker.records.size() > 0)
            {
                // Display the selected object's name.
                PickRecord const& record = mPicker.GetClosestNonnegative();
                mPickMessage = record.visual->name;
            }
            else
            {
                mPickMessage = "";
            }
        }
        return true;
    }

    return false;
}

bool CastleWindow3::SetEnvironment()
{
    // Set the search path to find images to load.
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/SceneGraphs/Castle/Data/Geometry/");
    mEnvironment.Insert(path + "/Samples/SceneGraphs/Castle/Data/Textures/");

    for (auto const& input : msGeometryInventory)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    for (auto const& input : msTextureInventory)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    return true;
}

void CastleWindow3::CreateScene()
{
    // Common vertex formats for the triangle meshes.
    mPNT1Format.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    mPNT1Format.Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0);
    mPNT1Format.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    mPNT2Format.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    mPNT2Format.Bind(VASemantic::NORMAL, DF_R32G32B32_FLOAT, 0);
    mPNT2Format.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    mPNT2Format.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 1);

    // Create the scene graph.  The translation is based on a priori knowledge
    // of the data set.
    mScene = std::make_shared<Node>();
    mScene->localTransform.SetTranslation(-1.3778250f, 0.70154405f, -2205.9973f);

    CreateLights();
    CreateEffects();
    CreateTextures();
    CreateSharedMeshes();

#if 1
    CreateWallTurret02();
    CreateWallTurret01();
    CreateWall02();
    CreateWall01();
    CreateQuadPatch01();
    CreateMainGate01();
    CreateMainGate();
    CreateExterior();
    CreateFrontHall();
    CreateFrontRamp();
    CreateDrawBridge();
    CreateCylinder02();
    CreateBridge();
    CreateLargePort();
    CreateSmallPort(1);
    CreateSmallPort(2);
    CreateRope(1);
    CreateRope(2);

    int32_t i;
    for (i = 1; i <= 7; ++i)
    {
        CreateWoodShield(i);
    }
    for (i = 1; i <= 17; ++i)
    {
        CreateTorch(i);
    }
    for (i = 1; i <= 3; ++i)
    {
        CreateKeg(i);
    }
    for (i = 2; i <= 37; ++i)
    {
        CreateBarrel(i);
    }
    for (i = 1; i <= 48; ++i)
    {
        CreateDoorFrame(i);
    }
    for (i = 49; i <= 60; ++i)
    {
        CreateDoorFramePivotTrn(i);
    }
    CreateDoorFrame(61);
    CreateDoorFrameScalePivotTrn(62);
    CreateDoorFrameScalePivotTrn(63);
    for (i = 64; i <= 68; ++i)
    {
        CreateDoorFrame(i);
    }
    for (i = 69; i <= 78; ++i)
    {
        CreateDoorFramePivotTrn(i);
    }
    CreateDoorFrame(79);
    CreateDoorFrameScalePivotTrn(80);
    CreateDoorFrameScalePivotTrn(81);
    CreateDoorFramePivotTrn(82);
    CreateDoorFramePivotTrn(83);
    CreateDoorFramePivotTrn(73);

    CreateBunk(1);
    for (i = 4; i <= 20; ++i)
    {
        CreateBunk(i);
    }
    for (i = 1; i <= 36; ++i)
    {
        CreateBench(i);
    }
    for (i = 1; i <= 9; ++i)
    {
        CreateTable(i);
    }
    for (i = 1; i <= 4; ++i)
    {
        CreateBarrelRack(i);
    }
    for (i = 1; i <= 36; ++i)
    {
        CreateChest(i);
    }
    for (i = 1; i <= 3; ++i)
    {
        CreateCeilingLight(i);
    }
    for (i = 1; i <= 7; ++i)
    {
        CreateSquareTable(i);
    }
    for (i = 1; i <= 27; ++i)
    {
        CreateSimpleChair(i);
    }
    for (i = 1; i <= 42; ++i)
    {
        CreateMug(i);
    }
    for (i = 1; i <= 9; ++i)
    {
        CreateDoor(i);
    }

    CreateTerrain();
    CreateSkyDome();
#endif
    CreateWater();
    CreateWater2();
}

void CastleWindow3::CreateLights()
{
    mDLight = std::make_shared<Light>(true, mEngine->HasDepthRange01());
    mDLight->lighting = std::make_shared<Lighting>();

    std::shared_ptr<ViewVolumeNode> lightNode = std::make_shared<ViewVolumeNode>(mDLight);
    lightNode->localTransform.SetTranslation(1628.448730f, -51.877197f, 0.0f);
    lightNode->localTransform.SetRotation(
        AxisAngle<4, float>({ -1.0f, 0.0f, 0.0f, 0.0f }, (float)GTE_C_HALF_PI));

    mDLightRoot = std::make_shared<Node>();
    mDLightRoot->localTransform.SetTranslation(-1824.998657f, -1531.269775f, 3886.592773f);
    mDLightRoot->localTransform.SetRotation(
        AxisAngle<4, float>({ -0.494124f, 0.325880f, 0.806005f }, 1.371538f));

    mDLightRoot->AttachChild(lightNode);
    mDLightRoot->Update();
}

void CastleWindow3::CreateEffects()
{
    std::shared_ptr<Material> common0 = std::make_shared<Material>();
    common0->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    common0->ambient = { 0.588235f, 0.588235f, 0.588235f, 1.0f };
    common0->diffuse = { 0.0f, 0.0f, 0.0f, 1.0f };
    common0->specular = { 0.0f, 0.0f, 0.0f, 2.0f };

    std::shared_ptr<Material> common1 = std::make_shared<Material>();
    common1->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    common1->ambient = { 0.213070f, 0.183005f, 0.064052f, 1.0f };
    common1->diffuse = { 0.0f, 0.0f, 0.0f, 1.0f };
    common1->specular = { 0.045f, 0.045f, 0.045f, 5.656854f };

    std::shared_ptr<Material> water = std::make_shared<Material>();
    water->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    water->ambient = { 0.088888f, 0.064052f, 0.181698f, 1.0f };
    water->diffuse = { 0.0f, 0.0f, 0.0f, 1.0f };
    water->specular = { 0.045f, 0.045f, 0.045f, 5.656854f };

    std::shared_ptr<Material> roofsteps = std::make_shared<Material>();
    roofsteps->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    roofsteps->ambient = { 0.1f, 0.1f, 0.1f, 1.0f };
    roofsteps->diffuse = { 0.0f, 0.0f, 0.0f, 1.0f };
    roofsteps->specular = { 0.045f, 0.045f, 0.045f, 5.656854f };

    // diffuse channel is outwall03.png
    mOutWallMaterial = common1;

    // diffuse channel is stone01.png
    mStoneMaterial = common1;

    // diffuse channel is river01.png (has alpha)
    mRiverMaterial = water;

    // emissive channel is walllightmap.png
    // diffuse channel is wall02.png
    mWallMaterial = common1;

    // emissive channel is walllightmap.png
    // diffuse channel is steps.png
    mStairsMaterial = roofsteps;

    // diffuse channel is outwall03.png
    mInteriorMaterial = common1;

    // emissive channel is walllightmap.png
    // diffuse channel is door.png
    mDoorMaterial = common0;

    // emissive channel is walllightmap.png
    // diffuse channel is floor02.png
    mFloorMaterial = common0;

    // emissive channel is walllightmap.png
    // diffuse channel is woodceiling.png
    mWoodCeilingMaterial = common0;

    // diffuse channel is keystone.png
    mKeystoneMaterial = common1;

    // diffuse channel is tileplanks.png
    mDrawBridgeMaterial = common1;

    // diffuse channel is rooftemp.png
    mRoofMaterial = roofsteps;

    // diffuse channel is ramp03.png
    mRampMaterial = common1;

    // diffuse channel is shield01.png
    mWoodShieldMaterial = common1;

    // diffuse channel is metal01.png
    mTorchHolderMaterial = std::make_shared<Material>();
    mTorchHolderMaterial->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    mTorchHolderMaterial->ambient = { 0.213070f, 0.183005f, 0.064052f, 1.0f };
    mTorchHolderMaterial->diffuse = { 0.0f, 0.0f, 0.0f, 1.0f };
    mTorchHolderMaterial->specular = { 0.216f, 0.216f, 0.216f, 11.313708f };

    // diffuse channel is torchwood.png
    mTorchWoodMaterial = common1;

    // emissive channel is torchhead.tga (same as .png ???)
    // diffuse channel is torchhead.png
    mTorchHeadMaterial = common0;

    // diffuse channel is barrelbase.png
    mBarrelBaseMaterial = common0;

    // diffuse channel is barrelbase.png
    mBarrelMaterial = common0;

    // emissive channel is walllightmap.png
    // diffuse channel is doorframe.png
    mDoorFrameMaterial = common1;

    // diffuse channel is bunkwood.png
    mBunkMaterial = common1;

    // diffuse channel is blanket.png
    mBlanketMaterial = common0;

    // diffuse channel is bunkwood.png
    mBenchMaterial = common0;

    // diffuse channel is bunkwood.png
    mTableMaterial = common0;

    mBarrelRackMaterial = mDrawBridgeMaterial;

    // diffuse channel is chest01.png
    mChestMaterial = common1;

    // diffuse channel is tileplanks.png
    mLightwoodMaterial = common1;

    // part of ceiling lights
    mMaterial26 = std::make_shared<Material>();
    mMaterial26->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    mMaterial26->ambient = { 0.588235f, 0.588235f, 0.588235f, 1.0f };
    mMaterial26->diffuse = { 0.588235f, 0.588235f, 0.588235f, 1.0f };
    mMaterial26->specular = { 0.0f, 0.0f, 0.0f, 2.0f };

    // diffuse channel is rope.png
    mRopeMaterial = common0;

    // diffuse channel is rope.png
    mSquareTableMaterial = common0;

    mSimpleChairMaterial = mDrawBridgeMaterial;

    // diffuse channel is mug.png
    mMugMaterial = common0;

    // diffuse channel is port.png
    mPortMaterial = common1;

    // diffuse channel is skyline.png
    mSkyMaterial = common0;

    // diffuse channel is river02.png (has alpha)
    mWaterMaterial = water;

    // TERRAIN
    // diffuse channel is gravel01.png
    mGravel1Material = common1;

    // diffuse channel is gravel02.png
    mGravel2Material = common1;

    // diffuse channel is gravel_corner_ne.png
    mGravelCornerNEMaterial = common1;

    // diffuse channel is gravel_corner_nw.png
    mGravelCornerNWMaterial = common1;

    // diffuse channel is gravel_corner_se.png
    mGravelCornerSEMaterial = common1;

    // diffuse channel is gravel_corner_sw.png
    mGravelCornerSWMaterial = common1;

    // diffuse channel is gravel_cap_ne.png
    mGravelCapNEMaterial = common1;

    // diffuse channel is gravel_cap_nw.png
    mGravelCapNWMaterial = common1;

    // diffuse channel is gravel_side_n.png
    mGravelSideNMaterial = common1;

    // diffuse channel is gravel_side_s.png
    mGravelSideSMaterial = common1;

    // diffuse channel is gravel_side_w.png
    mGravelSideWMaterial = common1;

    // diffuse channel is stone01.png
    mStone1Material = common1;

    // diffuse channel is stone02.png
    mStone2Material = common1;

    // diffuse channel is stone03.png
    mStone3Material = common1;

    // diffuse channel is largestone01.png
    mLargeStone1Material = common1;

    // diffuse channel is largerstone01.png
    mLargerStone1Material = common1;

    // diffuse channel is largerstone02.png
    mLargerStone2Material = common1;

    // diffuse channel is largeststone01.png
    mLargestStone1Material = common1;

    // diffuse channel is largeststone02.png
    mLargestStone2Material = common1;

    // diffuse channel is hugestone01.png
    mHugeStone1Material = common1;

    // diffuse channel is hugestone02.png
    mHugeStone2Material = common1;
}

void CastleWindow3::CreateTextures()
{
    std::string name = mEnvironment.GetPath("outwall03.png");
    mOutWall = WICFileIO::Load(name, true);
    mOutWall->AutogenerateMipmaps();

    name = mEnvironment.GetPath("stone01.png");
    mStone = WICFileIO::Load(name, true);
    mStone->AutogenerateMipmaps();

    name = mEnvironment.GetPath("river01.png");
    mRiver = WICFileIO::Load(name, true);
    mRiver->AutogenerateMipmaps();

    name = mEnvironment.GetPath("wall02.png");
    mWall = WICFileIO::Load(name, true);
    mWall->AutogenerateMipmaps();

    name = mEnvironment.GetPath("walllightmap.png");
    mWallLightMap = WICFileIO::Load(name, true);
    mWallLightMap->AutogenerateMipmaps();

    name = mEnvironment.GetPath("steps.png");
    mSteps = WICFileIO::Load(name, true);
    mSteps->AutogenerateMipmaps();

    name = mEnvironment.GetPath("door.png");
    mDoor = WICFileIO::Load(name, true);
    mDoor->AutogenerateMipmaps();

    name = mEnvironment.GetPath("floor02.png");
    mFloor = WICFileIO::Load(name, true);
    mFloor->AutogenerateMipmaps();

    name = mEnvironment.GetPath("woodceiling.png");
    mWoodCeiling = WICFileIO::Load(name, true);
    mWoodCeiling->AutogenerateMipmaps();

    name = mEnvironment.GetPath("keystone.png");
    mKeystone = WICFileIO::Load(name, true);
    mKeystone->AutogenerateMipmaps();

    name = mEnvironment.GetPath("tileplanks.png");
    mTilePlanks = WICFileIO::Load(name, true);
    mTilePlanks->AutogenerateMipmaps();

    name = mEnvironment.GetPath("rooftemp.png");
    mRoof = WICFileIO::Load(name, true);
    mRoof->AutogenerateMipmaps();

    name = mEnvironment.GetPath("ramp03.png");
    mRamp = WICFileIO::Load(name, true);
    mRamp->AutogenerateMipmaps();

    name = mEnvironment.GetPath("shield01.png");
    mShield = WICFileIO::Load(name, true);
    mShield->AutogenerateMipmaps();

    name = mEnvironment.GetPath("metal01.png");
    mMetal = WICFileIO::Load(name, true);
    mMetal->AutogenerateMipmaps();

    name = mEnvironment.GetPath("torchwood.png");
    mTorchWood = WICFileIO::Load(name, true);
    mTorchWood->AutogenerateMipmaps();

    name = mEnvironment.GetPath("torchhead.png");
    mTorchHead = WICFileIO::Load(name, true);
    mTorchHead->AutogenerateMipmaps();

    name = mEnvironment.GetPath("barrelbase.png");
    mBarrelBase = WICFileIO::Load(name, true);
    mBarrelBase->AutogenerateMipmaps();

    name = mEnvironment.GetPath("barrel.png");
    mBarrel = WICFileIO::Load(name, true);
    mBarrel->AutogenerateMipmaps();

    name = mEnvironment.GetPath("doorframe.png");
    mDoorFrame = WICFileIO::Load(name, true);
    mDoorFrame->AutogenerateMipmaps();

    name = mEnvironment.GetPath("bunkwood.png");
    mBunkwood = WICFileIO::Load(name, true);
    mBunkwood->AutogenerateMipmaps();

    name = mEnvironment.GetPath("blanket.png");
    mBlanket = WICFileIO::Load(name, true);
    mBlanket->AutogenerateMipmaps();

    mBench = mBunkwood;
    mTable = mBunkwood;
    mBarrelRack = mTilePlanks;

    name = mEnvironment.GetPath("chest01.png");
    mChest = WICFileIO::Load(name, true);
    mChest->AutogenerateMipmaps();

    mLightwood = mTilePlanks;

    name = mEnvironment.GetPath("rope.png");
    mRope = WICFileIO::Load(name, true);
    mRope->AutogenerateMipmaps();

    mSquareTable = mTilePlanks;
    mSimpleChair = mTilePlanks;

    name = mEnvironment.GetPath("mug.png");
    mMug = WICFileIO::Load(name, true);
    mMug->AutogenerateMipmaps();

    name = mEnvironment.GetPath("port.png");
    mPort = WICFileIO::Load(name, true);
    mPort->AutogenerateMipmaps();

    name = mEnvironment.GetPath("skyline.png");
    mSky = WICFileIO::Load(name, true);
    mSky->AutogenerateMipmaps();

    name = mEnvironment.GetPath("river02.png");
    mWater = WICFileIO::Load(name, true);
    mWater->AutogenerateMipmaps();

    // TERRAIN
    name = mEnvironment.GetPath("gravel01.png");
    mGravel1 = WICFileIO::Load(name, true);
    mGravel1->AutogenerateMipmaps();

    name = mEnvironment.GetPath("gravel02.png");
    mGravel2 = WICFileIO::Load(name, true);
    mGravel2->AutogenerateMipmaps();

    name = mEnvironment.GetPath("gravel_corner_se.png");
    mGravelCornerSE = WICFileIO::Load(name, true);
    mGravelCornerSE->AutogenerateMipmaps();

    name = mEnvironment.GetPath("gravel_corner_ne.png");
    mGravelCornerNE = WICFileIO::Load(name, true);
    mGravelCornerNE->AutogenerateMipmaps();

    name = mEnvironment.GetPath("gravel_corner_nw.png");
    mGravelCornerNW = WICFileIO::Load(name, true);
    mGravelCornerNW->AutogenerateMipmaps();

    name = mEnvironment.GetPath("gravel_corner_sw.png");
    mGravelCornerSW = WICFileIO::Load(name, true);
    mGravelCornerSW->AutogenerateMipmaps();

    mStone1 = mStone;

    name = mEnvironment.GetPath("stone02.png");
    mStone2 = WICFileIO::Load(name, true);
    mStone2->AutogenerateMipmaps();

    name = mEnvironment.GetPath("stone03.png");
    mStone3 = WICFileIO::Load(name, true);
    mStone3->AutogenerateMipmaps();

    name = mEnvironment.GetPath("gravel_cap_ne.png");
    mGravelCapNE = WICFileIO::Load(name, true);
    mGravelCapNE->AutogenerateMipmaps();

    name = mEnvironment.GetPath("gravel_cap_nw.png");
    mGravelCapNW = WICFileIO::Load(name, true);
    mGravelCapNW->AutogenerateMipmaps();

    name = mEnvironment.GetPath("gravel_side_s.png");
    mGravelSideS = WICFileIO::Load(name, true);
    mGravelSideS->AutogenerateMipmaps();

    name = mEnvironment.GetPath("gravel_side_n.png");
    mGravelSideN = WICFileIO::Load(name, true);
    mGravelSideN->AutogenerateMipmaps();

    name = mEnvironment.GetPath("gravel_side_w.png");
    mGravelSideW = WICFileIO::Load(name, true);
    mGravelSideW->AutogenerateMipmaps();

    name = mEnvironment.GetPath("largestone01.png");
    mLargeStone1 = WICFileIO::Load(name, true);
    mLargeStone1->AutogenerateMipmaps();

    name = mEnvironment.GetPath("largerstone01.png");
    mLargerStone1 = WICFileIO::Load(name, true);
    mLargerStone1->AutogenerateMipmaps();

    name = mEnvironment.GetPath("largerstone02.png");
    mLargerStone2 = WICFileIO::Load(name, true);
    mLargerStone2->AutogenerateMipmaps();

    name = mEnvironment.GetPath("largeststone01.png");
    mLargestStone1 = WICFileIO::Load(name, true);
    mLargestStone1->AutogenerateMipmaps();

    name = mEnvironment.GetPath("largeststone02.png");
    mLargestStone2 = WICFileIO::Load(name, true);
    mLargestStone2->AutogenerateMipmaps();

    name = mEnvironment.GetPath("hugestone01.png");
    mHugeStone1 = WICFileIO::Load(name, true);
    mHugeStone1->AutogenerateMipmaps();

    name = mEnvironment.GetPath("hugestone02.png");
    mHugeStone2 = WICFileIO::Load(name, true);
    mHugeStone2->AutogenerateMipmaps();

    name = mEnvironment.GetPath("gravel_corner_nw.png");
    mGravelCornerNW = WICFileIO::Load(name, true);
    mGravelCornerNW->AutogenerateMipmaps();

    name = mEnvironment.GetPath("gravel_corner_sw.png");
    mGravelCornerSW = WICFileIO::Load(name, true);
    mGravelCornerSW->AutogenerateMipmaps();
}

void CastleWindow3::CreateSharedMeshes()
{
    std::vector<std::shared_ptr<Visual>> meshes;

    mWoodShieldMesh = LoadMeshPNT1("WoodShield01.txt");

    mTorchMetalMesh = LoadMeshPNT1("Tube01.txt");
    meshes = LoadMeshPNT1Multi("Sphere01.txt");
    mTorchWoodMesh = meshes[0];
    mTorchHeadMesh = meshes[1];

    mVerticalSpoutMesh = LoadMeshPNT1("Cylinder03.txt");
    mHorizontalSpoutMesh = LoadMeshPNT1("Cylinder02NCL.txt");
    mBarrelHolderMesh = LoadMeshPNT1("Box01.txt");
    mBarrelMesh = LoadMeshPNT1("Barrel01.txt");
}

void CastleWindow3::InitializeFixedHeightRig()
{
    mCamera->SetFrustum(45.0f, GetAspectRatio(), 1.0f, 44495.0f);
    Vector4<float> camPosition{ 527.394f, 86.8992f, -2136.0f, 1.0f };
    Vector4<float> camDVector{ 1.0f, 0.0f, 0.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mFixedHeightRig.Set(mCamera, 0.5f, 0.001f);
    mFixedHeightRig.RegisterMoveForward(KEY_UP);
    mFixedHeightRig.RegisterMoveBackward(KEY_DOWN);
    mFixedHeightRig.RegisterTurnRight(KEY_RIGHT);
    mFixedHeightRig.RegisterTurnLeft(KEY_LEFT);
    mFixedHeightRig.RegisterLookUp(KEY_PAGE_UP);
    mFixedHeightRig.RegisterLookDown(KEY_PAGE_DOWN);
}

void CastleWindow3::UpdateVisualModelSpace(Spatial* object)
{
    Visual* visual = dynamic_cast<Visual*>(object);
    if (visual)
    {
        visual->UpdateModelBound();
        visual->UpdateModelNormals();
        return;
    }

    Node* node = dynamic_cast<Node*>(object);
    if (node)
    {
        for (int32_t i = 0; i < node->GetNumChildren(); ++i)
        {
            Spatial* child = node->GetChildPtr(i);
            if (child)
            {
                UpdateVisualModelSpace(child);
            }
        }
    }
}

void CastleWindow3::UpdateCameraLightModelPositions(Spatial* object)
{
    Visual* visual = dynamic_cast<Visual*>(object);
    if (visual)
    {
        VisualEffect* effect = visual->GetEffect().get();
#if defined(USE_DIRECTIONAL_LIGHT_TEXTURE)
        DirectionalLightTextureEffect* ltEffect =
            dynamic_cast<DirectionalLightTextureEffect*>(effect);
#else
        PointLightTextureEffect* ltEffect =
            dynamic_cast<PointLightTextureEffect*>(effect);
#endif
        if (ltEffect)
        {
            Matrix4x4<float> invWMatrix = visual->worldTransform.GetHInverse();
            auto const& geometry = ltEffect->GetGeometry();
#if defined(GTE_USE_MAT_VEC)
            geometry->lightModelPosition = invWMatrix * mDLight->GetPosition();
            geometry->lightModelDirection = invWMatrix * mDLight->GetDVector();
            geometry->cameraModelPosition = invWMatrix * mCamera->GetPosition();
#else
            geometry->lightModelPosition = mDLight->GetPosition() * invWMatrix;
            geometry->lightModelDirection = mDLight->GetDVector() * invWMatrix;
            geometry->cameraModelPosition = mCamera->GetPosition() * invWMatrix;
#endif
            Normalize(geometry->lightModelDirection);
            ltEffect->UpdateGeometryConstant();
        }
        return;
    }

    Node* node = dynamic_cast<Node*>(object);
    if (node)
    {
        for (int32_t i = 0; i < node->GetNumChildren(); ++i)
        {
            Spatial* child = node->GetChildPtr(i);
            if (child)
            {
                UpdateCameraLightModelPositions(child);
            }
        }
    }
}

void CastleWindow3::FixedHeightRig::SetPicker(std::shared_ptr<Node> const& scene, Picker& picker)
{
    mScene = scene;
    mPicker = &picker;
    mVerticalDistance = 5.0f;

    // Generate pick-ray information.
    double const multiplier = 1.0 / static_cast<double>(NUM_RAYS / 2);
    for (int32_t i = 0; i < NUM_RAYS; ++i)
    {
        double unit = multiplier * static_cast<double>(i) - 1.0;  // in [-1,1]
        float angle = static_cast<float>(GTE_C_HALF_PI + unit * GTE_C_QUARTER_PI);
        mCos[i] = std::cos(angle);
        mSin[i] = std::sin(angle);
        mTolerance[i] = static_cast<float>(2.0 - 1.5 * std::fabs(unit));  // in [1/2,1]
    }
}

void CastleWindow3::FixedHeightRig::SetVerticalDistance(float verticalDistance)
{
    mVerticalDistance = verticalDistance;
}

float CastleWindow3::FixedHeightRig::GetVerticalDistance() const
{
    return mVerticalDistance;
}

void CastleWindow3::FixedHeightRig::AdjustVerticalDistance()
{
    // Retain vertical distance above "ground".
    Vector4<float> pos = mCamera->GetPosition();
    Vector4<float> dir = -Vector4<float>::Unit(2);
    (*mPicker)(mScene, pos, dir, 0.0f, std::numeric_limits<float>::max());
    if (mPicker->records.size() > 0)
    {
        PickRecord const& record = mPicker->GetClosestNonnegative();
        Vector4<float> closest = record.primitivePoint - mVerticalDistance * dir;
        mCamera->SetPosition(closest);
    }
}

bool CastleWindow3::FixedHeightRig::AllowMotion(float sign)
{
    // Take a step forward or backward, depending on sign.  Check whether objects
    // are far enough away.  If so take the step.  If not, stay where you are.
    Vector4<float> pos = mCamera->GetPosition() + sign * mTranslationSpeed * mWorldAxis[0]
        - 0.5f * mVerticalDistance * mWorldAxis[1];

    for (int32_t i = 0; i < NUM_RAYS; ++i)
    {
        Vector4<float> dir = mCos[i] * mWorldAxis[2] + sign * mSin[i] * mWorldAxis[0];
        (*mPicker)(mScene, pos, dir, 0.0f, std::numeric_limits<float>::max());
        if (mPicker->records.size() > 0)
        {
            PickRecord const& record = mPicker->GetClosestNonnegative();
            if (record.distanceToLinePoint <= mTolerance[i])
            {
                return false;
            }
        }
    }

    return true;
}

void CastleWindow3::FixedHeightRig::MoveForward()
{
    if (AllowMotion(1.0f))
    {
        CameraRig::MoveForward();
        AdjustVerticalDistance();
    }
}

void CastleWindow3::FixedHeightRig::MoveBackward()
{
    if (AllowMotion(-1.0f))
    {
        CameraRig::MoveBackward();
        AdjustVerticalDistance();
    }
}

std::vector<std::string> const CastleWindow3::msGeometryInventory =
{
    "Barrel01.txt",
    "BarrelRack01.txt",
    "BarrelRack03.txt",
    "Bench01.txt",
    "Box01.txt",
    "Box02.txt",
    "Bridge.txt",
    "Bunk01.txt",
    "CeilingLight01.txt",
    "ChestBottom01.txt",
    "ChestTop01.txt",
    "Cylinder02.txt",
    "Cylinder02NCL.txt",
    "Cylinder03.txt",
    "Cylinder06.txt",
    "Door.txt",
    "DoorFrame01.txt",
    "DoorFrame53.txt",
    "DoorFrame61.txt",
    "DoorFrame62.txt",
    "DrawBridge.txt",
    "Exterior.txt",
    "FrontHall.txt",
    "FrontRamp.txt",
    "LargePort.txt",
    "MainGate.txt",
    "MainGate01.txt",
    "Mug.txt",
    "QuadPatch01.txt",
    "Rope.txt",
    "SimpleChair01.txt",
    "SkyDome.txt",
    "SmallPort.txt",
    "Sphere01.txt",
    "SquareTable01.txt",
    "Table01.txt",
    "Terrain.txt",
    "Tube01.txt",
    "Wall01.txt",
    "Wall02.txt",
    "WallTurret01.txt",
    "WallTurret02.txt",
    "Water.txt",
    "Water2.txt",
    "WoodShield01.txt"
};

std::vector<std::string> const CastleWindow3::msTextureInventory =
{
    "barrel.png",
    "barrelbase.png",
    "blanket.png",
    "bunkwood.png",
    "chest01.png",
    "door.png",
    "doorframe.png",
    "floor02.png",
    "gravel01.png",
    "gravel02.png",
    "gravel_cap_ne.png",
    "gravel_cap_nw.png",
    "gravel_corner_ne.png",
    "gravel_corner_nw.png",
    "gravel_corner_se.png",
    "gravel_corner_sw.png",
    "gravel_side_n.png",
    "gravel_side_s.png",
    "gravel_side_w.png",
    "hugestone01.png",
    "hugestone02.png",
    "keystone.png",
    "largerstone01.png",
    "largerstone02.png",
    "largestone01.png",
    "largeststone01.png",
    "largeststone02.png",
    "metal01.png",
    "mug.png",
    "outwall03.png",
    "port.png",
    "ramp03.png",
    "river01.png",
    "river02.png",
    "rooftemp.png",
    "rope.png",
    "shield01.png",
    "skyline.png",
    "steps.png",
    "stone01.png",
    "stone02.png",
    "stone03.png",
    "tileplanks.png",
    "torchhead.png",
    "torchwood.png",
    "wall02.png",
    "walllightmap.png",
    "woodceiling.png"
};
