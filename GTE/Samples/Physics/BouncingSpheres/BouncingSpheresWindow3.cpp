// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.14

#include "BouncingSpheresWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>
#include <Graphics/VertexColorEffect.h>
#include <fstream>

//#define SINGLE_STEP

BouncingSpheresWindow3::BouncingSpheresWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mModule{},
    mNoCullState{},
    mNoCullWireState{},
    mScene{},
    mPlaneMesh{},
    mSphereMesh{},
    mPhysicsTimer{},
    mPhysicsTime(0.0),
    mPhysicsDeltaTime(0.001)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetRasterizerState(mNoCullState);

    float angle = static_cast<float>(0.02 * GTE_C_PI);
    float cs = std::cos(angle), sn = std::sin(angle);
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.01f, 0.001f,
        { 64.0f, 0.0f, 20.0f }, { -cs, 0.0f, -sn }, { -sn, 0.0f, cs });

    CreateScene();

    // Initialize the balls with the correct transformations.
    PhysicsTick();
    GraphicsTick();
}

void BouncingSpheresWindow3::OnIdle()
{
    mTimer.Measure();
#if !defined(SINGLE_STEP)
    PhysicsTick();
#endif
    GraphicsTick();
    mTimer.UpdateFrameCount();
}
 
bool BouncingSpheresWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mNoCullWireState)
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        return true;

    case ' ':
#if defined(SINGLE_STEP)
        PhysicsTick();
#endif
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool BouncingSpheresWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/Physics/BouncingSpheres/");

    std::vector<std::string> inputs =
    {
        "BallTexture.png",
        "Initial.txt"
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

void BouncingSpheresWindow3::CreateScene()
{
    CreatePhysicsObjects();
    CreateGraphicsObjects();
}

void BouncingSpheresWindow3::CreatePhysicsObjects()
{
    // The file has 16 lines, each containing
    //   radius mass pos[0] pos[1] pos[2] linmom[0] linmom[1] linmom[2]
    mModule = std::make_unique<PhysicsModule>(
        NUM_SPHERES, -24.0, 24.0, -24.0, 24.0, 0.0, 40.0);

    std::string initialFile = mEnvironment.GetPath("Initial.txt");
    std::ifstream input(initialFile);
    double radius{}, mass{};
    Vector3<double> position{}, linearMomentum{};
    for (size_t i = 0; i < NUM_SPHERES; ++i)
    {
        input >> radius;
        input >> mass;
        input >> position[0] >> position[1] >> position[2];
        input >> linearMomentum[0] >> linearMomentum[1] >> linearMomentum[2];
        mModule->InitializeSphere(i, radius, mass, position, linearMomentum);
    }
    input.close();
}

void BouncingSpheresWindow3::CreateGraphicsObjects()
{
    // ** layout of scene graph **
    // trackball
    //     sceneNode
    //         floorMesh
    //         sidewall1Mesh
    //         sidewall2Mesh
    //         backwallMesh
    //         sphereMesh[0]
    //         :
    //         sphereMesh[15]

    mScene = std::make_shared<Node>();
    mTrackBall.Attach(mScene);

    // Create the walls.
    VertexFormat pcFormat{};
    pcFormat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    pcFormat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);

    // floor
    CreateWall(0, pcFormat,
        { -24.0f, -24.0f, 0.0f },
        { +24.0f, -24.0f, 0.0f },
        { +24.0f, +24.0f, 0.0f },
        { -24.0f, +24.0f, 0.0f },
        { 155.0f / 255.0f, 177.0f / 255.0f, 164.0f / 255.0f, 1.0f });

    // sidewall1
    CreateWall(1, pcFormat,
        { -24.0f, +24.0f, 0.0f },
        { +24.0f, +24.0f, 0.0f },
        { +24.0f, +24.0f, 40.0f },
        { -24.0f, +24.0f, 40.0f },
        { 170.0f / 255.0f, 187.0f / 255.0f, 219.0f / 255.0f, 1.0f });

    // sidewall2
    CreateWall(2, pcFormat,
        { +24.0f, -24.0f, 0.0f },
        { -24.0f, -24.0f, 0.0f },
        { -24.0f, -24.0f, 40.0f },
        { +24.0f, -24.0f, 40.0f },
        { 170.0f / 255.0f, 187.0f / 255.0f, 219.0f / 255.0f, 1.0f });

    // back wall
    CreateWall(3, pcFormat,
        { -24.0f, -24.0f, 0.0f },
        { -24.0f, +24.0f, 0.0f },
        { -24.0f, +24.0f, 40.0f },
        { -24.0f, -24.0f, 40.0f },
        { 209.0f / 255.0f, 204.0f / 255.0f, 180.0f / 255.0f, 1.0f });

    VertexFormat ptFormat{};
    ptFormat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    ptFormat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    // Create the spheres.
    MeshFactory mf{};
    mf.SetVertexFormat(ptFormat);
    std::string textureFile = mEnvironment.GetPath("BallTexture.png");
    auto texture = WICFileIO::Load(textureFile, false);
    for (size_t i = 0; i < NUM_SPHERES; ++i)
    {
        float radius = static_cast<float>(mModule->GetSphere(i).radius);
        auto mesh = mf.CreateSphere(16, 16, radius);
        auto effect = std::make_shared<Texture2Effect>(mProgramFactory,
            texture, SamplerState::Filter::MIN_L_MAG_L_MIP_P,
            SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);
        mesh->SetEffect(effect);
        mPVWMatrices.Subscribe(mesh);
        mSphereMesh[i] = mesh;
        mScene->AttachChild(mesh);
    }
}

void BouncingSpheresWindow3::CreateWall(
    size_t index, VertexFormat const& vformat,
    Vector3<float> const& pos0, Vector3<float> const& pos1,
    Vector3<float> const& pos2, Vector3<float> const& pos3,
    Vector4<float> const& color)
{
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    auto* vertices = vbuffer->Get<VertexPC>();
    vertices[0].position = pos0;
    vertices[1].position = pos1;
    vertices[2].position = pos2;
    vertices[3].position = pos3;
    vertices[0].color = color;
    vertices[1].color = color;
    vertices[2].color = color;
    vertices[3].color = color;

    auto ibuffer = std::make_shared<IndexBuffer>(IPType::IP_TRIMESH, 2, sizeof(uint32_t));
    ibuffer->SetTriangle(0, 0, 1, 2);
    ibuffer->SetTriangle(1, 0, 2, 3);

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);

    auto wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(wall);
    mPlaneMesh[index] = wall;
    mScene->AttachChild(wall);
}

void BouncingSpheresWindow3::PhysicsTick()
{
    mModule->DoTick(mPhysicsTime, mPhysicsDeltaTime);
    mPhysicsTime += mPhysicsDeltaTime;
}

void BouncingSpheresWindow3::GraphicsTick()
{
    // The graphics tick is called after the physics tick. The graphics
    // objects corresponding to the physical objects must be moved for
    // visualization.
    for (size_t i = 0; i < NUM_SPHERES; ++i)
    {
        Vector3<double> const& position = mModule->GetSphere(i).center;
        Vector3<float> translate
        {
            static_cast<float>(position[0]),
            static_cast<float>(position[1]),
            static_cast<float>(position[2])
        };
        mSphereMesh[i]->localTransform.SetTranslation(translate);
    }

    // Update the world transforms of the graphics objects.
    mTrackBall.Update();

    // Allow the user to move the camera of the scene.
    mCameraRig.Move();

    // The PVW matrices depend on the world transforms (W) of the
    // graphics objects and the projection-view transforms (PV) of
    // the camera.
    mPVWMatrices.Update();

    // Draw the scene, the frame rate and the simulation rate.
    mEngine->ClearBuffers();

    for (auto const& visual : mPlaneMesh)
    {
        mEngine->Draw(visual);
    }

    for (auto const& visual : mSphereMesh)
    {
        mEngine->Draw(visual);
    }

    std::array<float, 4> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    mEngine->Draw(8, mYSize - 8, black, mTimer.GetFPS());
    mEngine->Draw(90, mYSize - 8, black,
        std::string("Time = ") + std::to_string(mPhysicsTime));

    mEngine->DisplayColorBuffer(0);
}
