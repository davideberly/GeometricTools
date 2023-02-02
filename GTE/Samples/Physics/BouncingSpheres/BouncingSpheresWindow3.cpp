// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.06

#include "BouncingSpheresWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>
#include <Graphics/VertexColorEffect.h>
#include <fstream>

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
    mGraphicsTimer{},
    mLastPhysicsTime(0.0),
    mCurrPhysicsTime(0.0),
    mSimulationTime(0.0),
    mSimulationDeltaTime(0.001),
    mLastGraphicsTime(0.0),
    mCurrGraphicsTime(0.0),
    mSingleStep(false)
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

    // Initialize the spheres with the correct transformations.
    PhysicsTick();
    GraphicsTick();
}

void BouncingSpheresWindow3::OnIdle()
{
    if (!mSingleStep)
    {
        PhysicsTick();
    }
    GraphicsTick();
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
        if (mSingleStep)
        {
            PhysicsTick();
        }
        return true;
    case 's':
    case 'S':
        mSingleStep = !mSingleStep;
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
        "BallTextureWrap.png",
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
    mModule = std::make_unique<PhysicsModule>(
        NUM_SPHERES, -24.0, 24.0, -24.0, 24.0, 0.0, 40.0);

    std::string initialFile = mEnvironment.GetPath("Initial.txt");
    std::ifstream input(initialFile);
    double radius{}, massDensity{};
    Vector3<double> position{}, linearVelocity{}, angularVelocity{};
    Quaternion<double> qOrientation{};
    for (size_t i = 0; i < NUM_SPHERES; ++i)
    {
        input >> radius;
        input >> massDensity;
        input >> position[0] >> position[1] >> position[2];
        input >> linearVelocity[0] >> linearVelocity[1] >> linearVelocity[2];
        input >> qOrientation[0] >> qOrientation[1] >> qOrientation[2] >> qOrientation[3];
        input >> angularVelocity[0] >> angularVelocity[1] >> angularVelocity[2];
        mModule->InitializeSphere(i, radius, massDensity, position,
            linearVelocity, qOrientation, angularVelocity);
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
    std::string textureFile = mEnvironment.GetPath("BallTextureWrap.png");
    auto texture = WICFileIO::Load(textureFile, true);
    texture->AutogenerateMipmaps();
    for (size_t i = 0; i < NUM_SPHERES; ++i)
    {
        float radius = static_cast<float>(mModule->GetWorldSphere(i).radius);
        auto mesh = mf.CreateSphere(16, 16, radius);
        auto effect = std::make_shared<Texture2Effect>(mProgramFactory,
            texture, SamplerState::Filter::MIN_L_MAG_L_MIP_L,
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
    // Execute the physics system at 2400 frames per second, but use the
    // simulation time for a reproducible simulation.
    mCurrPhysicsTime = mPhysicsTimer.GetSeconds();
    double physicsDeltaTime = mCurrPhysicsTime - mLastPhysicsTime;
    if (physicsDeltaTime >= 1.0 / 2400.0)
    {
        mModule->DoTick(mSimulationTime, mSimulationDeltaTime);
        mSimulationTime += mSimulationDeltaTime;
        mLastPhysicsTime = mCurrPhysicsTime;
    }
}

void BouncingSpheresWindow3::GraphicsTick()
{
    // The graphics tick is called after the physics tick. The graphics
    // objects corresponding to the physical objects must be moved for
    // visualization. Execute the graphics system at 60 frames per second,
    // which allows the physics tick to run multiple times per frame.
    mCurrGraphicsTime = mGraphicsTimer.GetSeconds();
    double graphicsDeltaTime = mCurrGraphicsTime - mLastGraphicsTime;
    if (graphicsDeltaTime >= 1.0 / 60.0)
    {
        mTimer.Measure();
        for (size_t i = 0; i < NUM_SPHERES; ++i)
        {
            Vector3<double> const& position = mModule->GetWorldSphere(i).center;
            Vector3<float> translate
            {
                static_cast<float>(position[0]),
                static_cast<float>(position[1]),
                static_cast<float>(position[2])
            };
            mSphereMesh[i]->localTransform.SetTranslation(translate);

            Matrix3x3<double> const& orientation = mModule->GetOrientation(i);
            Matrix3x3<float> rotate{};
            for (int32_t r = 0; r < 3; ++r)
            {
                for (int32_t c = 0; c < 3; ++c)
                {
                    rotate(r, c) = static_cast<float>(orientation(r, c));
                }
            }
            mSphereMesh[i]->localTransform.SetRotation(rotate);
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
            std::string("Time = ") + std::to_string(mSimulationTime));

        mEngine->DisplayColorBuffer(0);
        mTimer.UpdateFrameCount();

        mLastGraphicsTime = mCurrGraphicsTime;
    }
}
