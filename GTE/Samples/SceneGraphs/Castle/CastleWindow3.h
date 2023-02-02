// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/Light.h>
#include <Graphics/ConstantColorEffect.h>
#include <Graphics/DirectionalLightTextureEffect.h>
#include <Graphics/PointLightTextureEffect.h>
#include <Graphics/Picker.h>
#include "TexturePNT1Effect.h"
using namespace gte;

// When exposed, turns off lighting so that the scene is unlit (texturing
// only).  TODO: This is a hack to make the light-texture effect become
// just a texture effect.  We need to determine the lighting model that
// was used in 3D Studio Max to create the castle scene.
#define DISABLE_LIGHTING

// When exposed, use a directional light to modulate the textures.  When not
// exposed, use a point light to modulate the textures.
//#define USE_DIRECTIONAL_LIGHT_TEXTURE

class CastleWindow3 : public Window3
{
public:
    CastleWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;
    virtual bool OnKeyDown(int32_t key, int32_t x, int32_t y) override;
    virtual bool OnKeyUp(int32_t key, int32_t x, int32_t y) override;
    virtual bool OnMouseClick(MouseButton button, MouseState state, int32_t x, int32_t y, uint32_t modifiers) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateLights();
    void CreateEffects();
    void CreateTextures();
    void CreateSharedMeshes();
    void InitializeFixedHeightRig();

    // The scene has many Visual objects.  The update of model bounds and
    // normals can be applied for each created object; however, it is
    // convenient to update all these by a single depth-first traversal of
    // the scene.
    void UpdateVisualModelSpace(Spatial* object);

    // For each mesh with a DirectionalLightTextureEffect, the update of
    // camera model position and light model position must occur each time
    // the camera moves.  Do so using a depth-first traversal of the scene.
    void UpdateCameraLightModelPositions(Spatial* object);

    std::shared_ptr<Node> mScene, mDLightRoot;
    std::shared_ptr<Light> mDLight;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Visual> mWaterMesh[2];
    Culler mCuller;

    std::shared_ptr<Visual> mSkyDome;

    // Picking support to allow collision avoidance and to display mesh
    // information.
    std::string mPickMessage;
    Picker mPicker;

    // Arrange for the camera to be a fixed distance above the nearest object.
    class FixedHeightRig : public CameraRig
    {
    public:
        FixedHeightRig()
            :
            CameraRig{},
            mScene{},
            mPicker(nullptr),
            mVerticalDistance(0.0f),
            mCos{},
            mSin{},
            mTolerance{}
        {
        }

        void SetPicker(std::shared_ptr<Node> const& scene, Picker& picker);
        void SetVerticalDistance(float verticalDistance);
        float GetVerticalDistance() const;
        void AdjustVerticalDistance();

    private:
        static int32_t constexpr NUM_RAYS = 5;

        bool AllowMotion(float sign);
        virtual void MoveForward();
        virtual void MoveBackward();

        std::shared_ptr<Node> mScene;
        Picker* mPicker;
        float mVerticalDistance;
        std::array<float, NUM_RAYS> mCos, mSin, mTolerance;
    };

    FixedHeightRig mFixedHeightRig;

    // Materials in the scene.
    std::shared_ptr<Material> mOutWallMaterial;
    std::shared_ptr<Material> mStoneMaterial;
    std::shared_ptr<Material> mRiverMaterial;
    std::shared_ptr<Material> mWallMaterial;
    std::shared_ptr<Material> mStairsMaterial;
    std::shared_ptr<Material> mInteriorMaterial;
    std::shared_ptr<Material> mDoorMaterial;
    std::shared_ptr<Material> mFloorMaterial;
    std::shared_ptr<Material> mWoodCeilingMaterial;
    std::shared_ptr<Material> mKeystoneMaterial;
    std::shared_ptr<Material> mDrawBridgeMaterial;
    std::shared_ptr<Material> mRoofMaterial;
    std::shared_ptr<Material> mRampMaterial;
    std::shared_ptr<Material> mWoodShieldMaterial;
    std::shared_ptr<Material> mTorchHolderMaterial;
    std::shared_ptr<Material> mTorchWoodMaterial;
    std::shared_ptr<Material> mTorchHeadMaterial;
    std::shared_ptr<Material> mBarrelBaseMaterial;
    std::shared_ptr<Material> mBarrelMaterial;
    std::shared_ptr<Material> mDoorFrameMaterial;
    std::shared_ptr<Material> mBunkMaterial;
    std::shared_ptr<Material> mBlanketMaterial;
    std::shared_ptr<Material> mBenchMaterial;
    std::shared_ptr<Material> mTableMaterial;
    std::shared_ptr<Material> mBarrelRackMaterial;
    std::shared_ptr<Material> mChestMaterial;
    std::shared_ptr<Material> mLightwoodMaterial;
    std::shared_ptr<Material> mMaterial26;
    std::shared_ptr<Material> mRopeMaterial;
    std::shared_ptr<Material> mSquareTableMaterial;
    std::shared_ptr<Material> mSimpleChairMaterial;
    std::shared_ptr<Material> mMugMaterial;
    std::shared_ptr<Material> mPortMaterial;
    std::shared_ptr<Material> mSkyMaterial;
    std::shared_ptr<Material> mWaterMaterial;
    std::shared_ptr<Material> mGravel1Material;
    std::shared_ptr<Material> mGravel2Material;
    std::shared_ptr<Material> mGravelCornerNEMaterial;
    std::shared_ptr<Material> mGravelCornerNWMaterial;
    std::shared_ptr<Material> mGravelCornerSEMaterial;
    std::shared_ptr<Material> mGravelCornerSWMaterial;
    std::shared_ptr<Material> mGravelCapNEMaterial;
    std::shared_ptr<Material> mGravelCapNWMaterial;
    std::shared_ptr<Material> mGravelSideNMaterial;
    std::shared_ptr<Material> mGravelSideSMaterial;
    std::shared_ptr<Material> mGravelSideWMaterial;
    std::shared_ptr<Material> mStone1Material;
    std::shared_ptr<Material> mStone2Material;
    std::shared_ptr<Material> mStone3Material;
    std::shared_ptr<Material> mLargeStone1Material;
    std::shared_ptr<Material> mLargerStone1Material;
    std::shared_ptr<Material> mLargerStone2Material;
    std::shared_ptr<Material> mLargestStone1Material;
    std::shared_ptr<Material> mLargestStone2Material;
    std::shared_ptr<Material> mHugeStone1Material;
    std::shared_ptr<Material> mHugeStone2Material;

    // Textures in the scene.
    std::shared_ptr<Texture2> mOutWall;
    std::shared_ptr<Texture2> mStone;
    std::shared_ptr<Texture2> mRiver;
    std::shared_ptr<Texture2> mWall;
    std::shared_ptr<Texture2> mWallLightMap;
    std::shared_ptr<Texture2> mSteps;
    std::shared_ptr<Texture2> mDoor;
    std::shared_ptr<Texture2> mFloor;
    std::shared_ptr<Texture2> mWoodCeiling;
    std::shared_ptr<Texture2> mKeystone;
    std::shared_ptr<Texture2> mTilePlanks;
    std::shared_ptr<Texture2> mRoof;
    std::shared_ptr<Texture2> mRamp;
    std::shared_ptr<Texture2> mShield;
    std::shared_ptr<Texture2> mMetal;
    std::shared_ptr<Texture2> mTorchWood;
    std::shared_ptr<Texture2> mTorchHead;
    std::shared_ptr<Texture2> mBarrelBase;
    std::shared_ptr<Texture2> mBarrel;
    std::shared_ptr<Texture2> mDoorFrame;
    std::shared_ptr<Texture2> mBunkwood;
    std::shared_ptr<Texture2> mBlanket;
    std::shared_ptr<Texture2> mBench;
    std::shared_ptr<Texture2> mTable;
    std::shared_ptr<Texture2> mBarrelRack;
    std::shared_ptr<Texture2> mChest;
    std::shared_ptr<Texture2> mLightwood;
    std::shared_ptr<Texture2> mRope;
    std::shared_ptr<Texture2> mSquareTable;
    std::shared_ptr<Texture2> mSimpleChair;
    std::shared_ptr<Texture2> mMug;
    std::shared_ptr<Texture2> mPort;
    std::shared_ptr<Texture2> mSky;
    std::shared_ptr<Texture2> mWater;
    std::shared_ptr<Texture2> mGravel1;
    std::shared_ptr<Texture2> mGravel2;
    std::shared_ptr<Texture2> mGravelCornerNE;
    std::shared_ptr<Texture2> mGravelCornerNW;
    std::shared_ptr<Texture2> mGravelCornerSE;
    std::shared_ptr<Texture2> mGravelCornerSW;
    std::shared_ptr<Texture2> mGravelCapNE;
    std::shared_ptr<Texture2> mGravelCapNW;
    std::shared_ptr<Texture2> mGravelSideN;
    std::shared_ptr<Texture2> mGravelSideS;
    std::shared_ptr<Texture2> mGravelSideW;
    std::shared_ptr<Texture2> mStone1;
    std::shared_ptr<Texture2> mStone2;
    std::shared_ptr<Texture2> mStone3;
    std::shared_ptr<Texture2> mLargeStone1;
    std::shared_ptr<Texture2> mLargerStone1;
    std::shared_ptr<Texture2> mLargerStone2;
    std::shared_ptr<Texture2> mLargestStone1;
    std::shared_ptr<Texture2> mLargestStone2;
    std::shared_ptr<Texture2> mHugeStone1;
    std::shared_ptr<Texture2> mHugeStone2;

    // Shared meshes.
    std::shared_ptr<Visual> mWoodShieldMesh;
    std::shared_ptr<Visual> mTorchMetalMesh;
    std::shared_ptr<Visual> mTorchWoodMesh;
    std::shared_ptr<Visual> mTorchHeadMesh;
    std::shared_ptr<Visual> mVerticalSpoutMesh;
    std::shared_ptr<Visual> mHorizontalSpoutMesh;
    std::shared_ptr<Visual> mBarrelHolderMesh;
    std::shared_ptr<Visual> mBarrelMesh;
    std::shared_ptr<Visual> mDoorFrame01Mesh;
    std::shared_ptr<Visual> mDoorFrame53Mesh;
    std::shared_ptr<Visual> mDoorFrame61Mesh;
    std::shared_ptr<Visual> mDoorFrame62Mesh;

private:
    // Support for loading data sets.
    class LookupPNT1
    {
    public:
        LookupPNT1();
        bool operator< (LookupPNT1 const& other) const;
        int32_t PIndex, NIndex, TIndex;
    };

    struct VertexPNT1
    {
        Vector3<float> position, normal;
        Vector2<float> tcoord;

        bool operator<(VertexPNT1 const& v) const
        {
            if (position < v.position)
            {
                return true;
            }
            if (position > v.position)
            {
                return false;
            }
            if (normal < v.normal)
            {
                return true;
            }
            if (normal > v.normal)
            {
                return false;
            }
            return tcoord < v.tcoord;
        }
    };

    class LookupPNT2
    {
    public:
        LookupPNT2();
        bool operator< (LookupPNT2 const& other) const;
        int32_t PIndex, NIndex, T0Index, T1Index;
    };

    struct VertexPNT2
    {
        Vector3<float> position, normal;
        Vector2<float> tcoord0, tcoord1;

        bool operator<(VertexPNT2 const& v) const
        {
            if (position < v.position)
            {
                return true;
            }
            if (position > v.position)
            {
                return false;
            }
            if (normal < v.normal)
            {
                return true;
            }
            if (normal > v.normal)
            {
                return false;
            }
            if (tcoord0 < v.tcoord0)
            {
                return true;
            }
            if (tcoord0 > v.tcoord0)
            {
                return false;
            }
            return tcoord1 < v.tcoord1;
        }
    };

    std::shared_ptr<Visual> LoadMeshPNT1(std::string const& name);
    std::shared_ptr<Visual> LoadMeshPNT2(std::string const& name);
    std::vector<std::shared_ptr<Visual>> LoadMeshPNT1Multi(std::string const& name);
    void GetTuple3(std::ifstream& inFile, std::vector<Vector3<float>>& elements);
    void GetTuple2(std::ifstream& inFile, std::vector<Vector2<float>>& elements);

    VertexFormat mPNT1Format, mPNT2Format;

private:
    // The list of exported objects.
    void CreateWallTurret02();
    void CreateWallTurret01();
    void CreateWall02();
    void CreateWall01();
    void CreateQuadPatch01();
    void CreateWater();
    void CreateWater2();
    void CreateMainGate01();
    void CreateMainGate();
    void CreateFrontHall();
    void CreateFrontRamp();
    void CreateExterior();
    void CreateDrawBridge();
    void CreateCylinder02();
    void CreateBridge();
    void CreateLargePort();
    void CreateSmallPort(int32_t i);
    void CreateRope(int32_t i);
    void CreateSkyDome();
    void CreateTerrain();

    enum { MAX_WOODSHIELDS = 8 };  // 0 unused
    static Vector4<float> msWoodShieldTrn[MAX_WOODSHIELDS];
    static float msWoodShieldYRotate[MAX_WOODSHIELDS];
    static float msWoodShieldXRotate[MAX_WOODSHIELDS];
    void CreateWoodShield(int32_t i);

    enum { MAX_TORCHES = 18 };  // 0 unused
    static Vector4<float> msTorchTrn[MAX_TORCHES];
    static float msTorchZAngle[MAX_TORCHES];
    void CreateTorch(int32_t i);

    enum { MAX_KEGS = 4 };  // 0 unused
    static Vector4<float> msKegTrn[MAX_KEGS];
    static float msKegZAngle[MAX_KEGS];
    void CreateKeg(int32_t i);

    enum { MAX_BARRELS = 38 };  // 0 and 1 not used
    static Vector4<float> msBarrelTrn[MAX_BARRELS];
    static float msBarrelZAngle[MAX_BARRELS];
    static float msBarrelYAngle[MAX_BARRELS];
    static float msBarrelXAngle[MAX_BARRELS];
    void CreateBarrel(int32_t i);

    enum { MAX_DOORFRAMES = 84 };  // 0 unused
    static Vector4<float> msDoorFrameTrn[MAX_DOORFRAMES];
    static float msDoorFrameZAngle[MAX_DOORFRAMES];
    static Vector4<float> msDoorFramePivotTrn[35];  // index i-49
    std::string GetDoorFrameFilename(int32_t i);
    void CreateDoorFrame(int32_t i);  // 1 <= i <= 48, i = 61, 64 <= i <= 68, 79
    void CreateDoorFramePivotTrn(int32_t i);  // 49 <= i <= 60, 69 <= i <= 78, 82, 83
    void CreateDoorFrameScalePivotTrn(int32_t i);  // 62, 63, 80, 81

    enum { MAX_BUNKS = 21 };  // 0, 2, 3 unused
    static Vector4<float> msBunkTrn[MAX_BUNKS];
    static float msBunkZAngle[MAX_BUNKS];
    void CreateBunk(int32_t i);

    enum { MAX_BENCHES = 37 };  // 0 unused
    static Vector4<float> msBenchTrn[MAX_BENCHES];
    static float msBenchZAngle[MAX_BENCHES];
    void CreateBench(int32_t i);

    enum { MAX_TABLES = 10 };  // 0 unused
    static Vector4<float> msTableTrn[MAX_TABLES];
    static float msTableZAngle[MAX_TABLES];
    void CreateTable(int32_t i);

    enum { MAX_BARREL_RACKS = 5 };  // 0 unused
    static Vector4<float> msBarrelRackTrn[MAX_BARREL_RACKS];
    void CreateBarrelRack(int32_t i);

    enum { MAX_CHESTS = 37 };  // 0 unused
    static Vector4<float> msChestTrn[MAX_CHESTS];
    static float msChestZAngle[MAX_CHESTS];
    void CreateChest(int32_t i);

    enum { MAX_CEILING_LIGHTS = 4 };  // 0 unused
    static Vector4<float> msCeilingLightTrn[MAX_CEILING_LIGHTS];
    void CreateCeilingLight(int32_t i);

    enum { MAX_SQUARE_TABLES = 8 };  // 0 unused
    static Vector4<float> msSquareTableTrn[MAX_SQUARE_TABLES];
    static float msSquareTableZAngle[MAX_SQUARE_TABLES];
    void CreateSquareTable(int32_t i);

    enum { MAX_SIMPLE_CHAIRS = 28 };  // 0 unused
    static Vector4<float> msSimpleChairTrn[MAX_SIMPLE_CHAIRS];
    static float msSimpleChairZAngle[MAX_SIMPLE_CHAIRS];
    void CreateSimpleChair(int32_t i);

    enum { MAX_MUGS = 43 };  // 0 unused
    static Vector4<float> msMugTrn[MAX_MUGS];
    static float msMugZAngle[MAX_MUGS];
    void CreateMug(int32_t i);

    enum { MAX_DOORS = 10 };  // 0 unused
    static Vector4<float> msDoorTrn[MAX_DOORS];
    static float msDoorZAngle[MAX_DOORS];
    void CreateDoor(int32_t i);

#if defined(USE_DIRECTIONAL_LIGHT_TEXTURE)
    std::shared_ptr<DirectionalLightTextureEffect> CreateLTEffect(
        std::shared_ptr<Material> const& material, std::shared_ptr<Texture2> const& texture);
#else
    std::shared_ptr<PointLightTextureEffect> CreateLTEffect(
        std::shared_ptr<Material> const& material, std::shared_ptr<Texture2> const& texture);
#endif

    std::shared_ptr<TexturePNT1Effect> CreateTextureEffect(
        std::shared_ptr<Texture2> const& texture);

    std::shared_ptr<ConstantColorEffect> CreateMaterialEffect(
        std::shared_ptr<Material> const& material);

    static std::vector<std::string> const msGeometryInventory;
    static std::vector<std::string> const msTextureInventory;
};
