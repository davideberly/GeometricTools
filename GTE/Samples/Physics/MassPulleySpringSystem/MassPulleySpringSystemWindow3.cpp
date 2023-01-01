// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "MassPulleySpringSystemWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>
#include <Graphics/Texture2Effect.h>

MassPulleySpringSystemWindow3::MassPulleySpringSystemWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mLastUpdateTime(mMotionTimer.GetSeconds())
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    mEngine->SetClearColor({ 0.819607f, 0.909803f, 0.713725f, 1.0f });

    InitializeModule();
    CreateScene();

    float angle = static_cast<float>(0.1 * GTE_C_PI);
    float cs = std::cos(angle), sn = std::sin(angle);
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.1f, 0.001f,
        { 0.0f, 48.0f, 326.0f }, { 0.0f, sn, -cs }, { 0.0f, -cs, -sn });
    mPVWMatrices.Update();
}

void MassPulleySpringSystemWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    double time = mMotionTimer.GetSeconds();
    if (30.0 * (time - mLastUpdateTime) >= 1.0)
    {
        mLastUpdateTime = time;
#if !defined(MASS_PULLEY_SPRING_SYSTEM_SINGLE_STEP)
        PhysicsTick();
#endif
        GraphicsTick();
    }

    mTimer.UpdateFrameCount();
}

bool MassPulleySpringSystemWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mWireState == mEngine->GetRasterizerState())
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;

    case 'i':
    case 'I':
        InitializeModule();
        return true;

#if defined(MASS_PULLEY_SPRING_SYSTEM_SINGLE_STEP)
    case 'g':
    case 'G':
        PhysicsTick();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool MassPulleySpringSystemWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "Metal.png",
        "Rope.png",
        "Wood.png"
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

void MassPulleySpringSystemWindow3::InitializeModule()
{
    mModule.gravity = 1.0f;
    mModule.mass1 = 1.0f;
    mModule.mass2 = 2.0f;
    mModule.mass3 = 3.0f;
    mModule.radius = 32.0f;
    mModule.inertia = std::pow(mModule.radius, 4.0f) * (float)GTE_C_HALF_PI;
    mModule.wireLength = 375.0f + mModule.radius * (float)GTE_C_PI;
    mModule.springLength = 100.0f;
    mModule.springConstant = 10.0f;

    float time = 0.0f;
    float deltaTime = 0.1f;
    float y1 = 200.0f;
    float dy1 = -10.0f;
    float dy3 = -20.0f;
    mModule.Initialize(time, deltaTime, y1, dy1, dy3);
}

void MassPulleySpringSystemWindow3::CreateScene()
{
    // set up the scene graph
    // scene -+- floor
    //        |
    //        +- assembly -+- cableRoot -+- cable
    //                     |             |
    //                     |             +- mass0
    //                     |             |
    //                     |             +- mass1
    //                     |
    //                     +- pulleyRoot -+- pulley -+- plate0
    //                                    |          |
    //                                    |          +- plate1
    //                                    |          |
    //                                    |          +- cylinder
    //                                    |
    //                                    +- spring -+- side0
    //                                               |
    //                                               +- side1
    //                                               |
    //                                               +- top
    //                                               |
    //                                               +- wire

    mScene = std::make_shared<Node>();
    mAssembly = std::make_shared<Node>();
    mCableRoot = std::make_shared<Node>();
    mPulleyRoot = std::make_shared<Node>();
    mPulley = std::make_shared<Node>();
    mSpring = std::make_shared<Node>();

    CreateFloor();
    CreateCable();
    mMass1 = CreateMass(1.0f);
    mMass2 = CreateMass(2.0f);
    CreatePulley();
    CreateSpring();
    CreateHelix();

    mScene->AttachChild(mFloor);
    mScene->AttachChild(mAssembly);
    mAssembly->AttachChild(mCableRoot);
    mAssembly->AttachChild(mPulleyRoot);
    mCableRoot->AttachChild(mCable);
    mCableRoot->AttachChild(mMass1);
    mCableRoot->AttachChild(mMass2);
    mPulleyRoot->AttachChild(mPulley);
    mPulleyRoot->AttachChild(mSpring);
    mPulley->AttachChild(mPlate0);
    mPulley->AttachChild(mPlate1);
    mPulley->AttachChild(mCylinder);
    mSpring->AttachChild(mSide0);
    mSpring->AttachChild(mSide1);
    mSpring->AttachChild(mTop);
    mSpring->AttachChild(mHelix);

    mPulleyRoot->localTransform.SetTranslation(0.0f, mModule.GetCurrentY3(), 0.0f);

    UpdateCable();
    UpdateHelix();
    mScene->Update();

    mVisuals.resize(11);
    mVisuals[0] = mFloor;
    mVisuals[1] = mCable;
    mVisuals[2] = mMass1;
    mVisuals[3] = mMass2;
    mVisuals[4] = mPlate0;
    mVisuals[5] = mPlate1;
    mVisuals[6] = mCylinder;
    mVisuals[7] = mSide0;
    mVisuals[8] = mSide1;
    mVisuals[9] = mTop;
    mVisuals[10] = mHelix;
}

void MassPulleySpringSystemWindow3::CreateFloor()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mFloor = mf.CreateRectangle(2, 2, 1024.0f, 1024.0f);
    auto const& vbuffer = mFloor->GetVertexBuffer();
    uint32_t const numVertices = vbuffer->GetNumElements();
    auto vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        Vector3<float> position = vertices[i].position;
        vertices[i].position = { position[1], 255.0f - position[2], -position[0] };
    }

    auto texture = WICFileIO::Load(mEnvironment.GetPath("Wood.png"), false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);
    mFloor->SetEffect(effect);
    mPVWMatrices.Subscribe(mFloor->worldTransform, effect->GetPVWMatrixConstant());
}

void MassPulleySpringSystemWindow3::CreateCable()
{
    MeshDescription desc(MeshTopology::CYLINDER, 128, 16);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, desc.numVertices);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto vertices = vbuffer->Get<Vertex>();
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, desc.numTriangles, sizeof(uint32_t));

    desc.vertexAttributes =
    {
        VertexAttribute("position", &vertices[0].position, sizeof(Vertex)),
        VertexAttribute("tcoord", &vertices[0].tcoord, sizeof(Vertex))
    };

    desc.indexAttribute = IndexAttribute(ibuffer->GetData(), ibuffer->GetElementSize());

    // Create a quadratic spline for the medial axis.  The control points are
    // initially zero, but the UpdateCable() function will fill them in with
    // those points needed to define the cable.
    BasisFunctionInput<float> input(1024, 2);
    mCableSpline = std::make_shared<BSplineCurve<3, float>>(input, nullptr);
    Vector3<float> zero{ 0.0f, 0.0f, 0.0f };
    for (int32_t i = 0; i < mCableSpline->GetNumControls(); ++i)
    {
        mCableSpline->SetControl(i, zero);
    }

    // Generate a tube surface whose medial axis is the spline.
    mCableSurface = std::make_unique<TubeMesh<float>>(desc, mCableSpline,
        [](float) { return 0.5f; }, false, false, Vector3<float>{ 0.0f, 0.0f, 1.0f });

    auto texture = WICFileIO::Load(mEnvironment.GetPath("Rope.png"), false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);
    mCable = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mCable->worldTransform, effect->GetPVWMatrixConstant());
}

std::shared_ptr<Visual> MassPulleySpringSystemWindow3::CreateMass(float radius)
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    auto mass = mf.CreateSphere(8, 8, radius);

    Vector4<float> gray{ 0.75f, 0.75f, 0.75f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, gray);
    mass->SetEffect(effect);
    mPVWMatrices.Subscribe(mass->worldTransform, effect->GetPVWMatrixConstant());
    return mass;
}

void MassPulleySpringSystemWindow3::CreatePulley()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    float const thickness = 4.0f;

    mPlate0 = mf.CreateDisk(4, 32, mModule.radius);
    mPlate0->localTransform.SetTranslation(0.0f, 0.0f, 0.5f * thickness);
    auto texture = WICFileIO::Load(mEnvironment.GetPath("Metal.png"), false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);
    mPlate0->SetEffect(effect);
    mPVWMatrices.Subscribe(mPlate0->worldTransform, effect->GetPVWMatrixConstant());

    mPlate1 = mf.CreateDisk(4, 32, mModule.radius);
    mPlate1->localTransform.SetTranslation(0.0f, 0.0f, 0.5f * thickness);
    auto const& vbuffer = mPlate1->GetVertexBuffer();
    uint32_t const numVertices = vbuffer->GetNumElements();
    auto vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].position[0] = -vertices[i].position[0];
    }
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);
    mPlate1->SetEffect(effect);
    mPVWMatrices.Subscribe(mPlate1->worldTransform, effect->GetPVWMatrixConstant());

    mCylinder = mf.CreateCylinderOpen(2, 32, mModule.radius, thickness);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);
    mCylinder->SetEffect(effect);
    mPVWMatrices.Subscribe(mCylinder->worldTransform, effect->GetPVWMatrixConstant());
}

void MassPulleySpringSystemWindow3::CreateSpring()
{
    float const thickness = 4.0f;
    float xExtent = 2.0f;
    float yExtent = 18.0f;
    float zExtent = 1.0f;
    Vector4<float> black{ 0.0f, 0.0f, 0.0f, 1.0f };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mSide0 = mf.CreateBox(xExtent, yExtent, zExtent);
    std::shared_ptr<VertexBuffer> vbuffer = mSide0->GetVertexBuffer();
    uint32_t numVertices = vbuffer->GetNumElements();
    auto vertices = vbuffer->Get<Vector3<float>>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i][1] -= 0.5f * mModule.radius;
        vertices[i][2] += 0.5f * thickness + zExtent;
    }
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, black);
    mSide0->SetEffect(effect);
    mPVWMatrices.Subscribe(mSide0->worldTransform, effect->GetPVWMatrixConstant());

    mSide1 = mf.CreateBox(xExtent, yExtent, zExtent);
    vbuffer = mSide1->GetVertexBuffer();
    numVertices = vbuffer->GetNumElements();
    vertices = vbuffer->Get<Vector3<float>>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i][0] = -vertices[i][0];
        vertices[i][1] -= 0.5f * mModule.radius;
        vertices[i][2] = -vertices[i][2] - 0.5f * thickness - zExtent;
    }
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, black);
    mSide1->SetEffect(effect);
    mPVWMatrices.Subscribe(mSide1->worldTransform, effect->GetPVWMatrixConstant());

    yExtent = xExtent;
    xExtent = 0.5f * thickness + 2.0f;
    zExtent = 1.0f;
    mTop = mf.CreateBox(xExtent, yExtent, zExtent);
    vbuffer = mTop->GetVertexBuffer();
    numVertices = vbuffer->GetNumElements();
    vertices = vbuffer->Get<Vector3<float>>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        Vector3<float> position = vertices[i];
        vertices[i][0] = position[2];
        vertices[i][1] = position[0] - 0.5f * mModule.radius - yExtent - 0.5f;
        vertices[i][2] = position[1];
    }
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, black);
    mTop->SetEffect(effect);
    mPVWMatrices.Subscribe(mTop->worldTransform, effect->GetPVWMatrixConstant());
}

void MassPulleySpringSystemWindow3::CreateHelix()
{
    MeshDescription desc(MeshTopology::CYLINDER, 128, 16);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, desc.numVertices);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto vertices = vbuffer->Get<Vertex>();
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, desc.numTriangles, sizeof(uint32_t));

    desc.vertexAttributes =
    {
        VertexAttribute("position", &vertices[0].position, sizeof(Vertex)),
        VertexAttribute("tcoord", &vertices[0].tcoord, sizeof(Vertex))
    };

    desc.indexAttribute = IndexAttribute(ibuffer->GetData(), ibuffer->GetElementSize());

    // Create a quadratic spline for the medial axis.  The control points are
    // initially zero, but the UpdateHelix() function will fill them in with
    // those points needed to define the helix.
    BasisFunctionInput<float> input(1024, 2);
    mHelixSpline = std::make_shared<BSplineCurve<3, float>>(input, nullptr);
    Vector3<float> zero{ 0.0f, 0.0f, 0.0f };
    for (int32_t i = 0; i < mHelixSpline->GetNumControls(); ++i)
    {
        mHelixSpline->SetControl(i, zero);
    }

    // Generate a tube surface whose medial axis is the spline.
    mHelixSurface = std::make_unique<TubeMesh<float>>(desc, mHelixSpline,
        [](float) { return 0.25f; }, false, false, Vector3<float>{ 0.0f, 0.0f, 1.0f });

    auto texture = WICFileIO::Load(mEnvironment.GetPath("Metal.png"), false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);
    mHelix = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mHelix->worldTransform, effect->GetPVWMatrixConstant());
}

void MassPulleySpringSystemWindow3::UpdatePulley()
{
    AxisAngle<4, float> aa(Vector4<float>::Unit(2), mModule.GetAngle());
    mPulley->localTransform.SetRotation(aa);
    mPulleyRoot->localTransform.SetTranslation(0.0f, mModule.GetCurrentY3(), 0.0f);
}

void MassPulleySpringSystemWindow3::UpdateCable()
{
    // Partition control points between two vertical wires and circle piece.
    int32_t const numCtrls = mCableSpline->GetNumControls();
    float const fraction1 = mModule.GetCableFraction1();
    float const fraction2 = mModule.GetCableFraction2();
    float const fractionC = 1.0f - fraction1 - fraction2;

    int32_t imin, imax, i;
    float mult, t;
    Vector3<float> ctrl{ 0.0f, 0.0f, 0.0f };

    // Set control points for wire from mass 1 to pulley midline.
    imin = 0;
    imax = static_cast<int32_t>(fraction1 * numCtrls);
    if (imin < imax)
    {
        mult = 1.0f / static_cast<float>(imax - imin);
        ctrl[0] = -mModule.radius;
        for (i = imin; i <= imax; ++i)
        {
            t = mult * static_cast<float>(i - imin);
            ctrl[1] = (1.0f - t) * mModule.GetCurrentY1() + t * mModule.GetCurrentY3();
            mCableSpline->SetControl(i, ctrl);
        }
    }
    else
    {
        mCableSpline->SetControl(imin, ctrl);
    }

    // Set control points for wire along hemicircle of pulley.
    imin = imax + 1;
    imax += static_cast<int32_t>(fractionC * numCtrls);
    mult = 1.0f / static_cast<float>(imax - imin);
    for (i = imin; i <= imax; ++i)
    {
        t = -1.0f + mult * static_cast<float>(i - imin);
        float angle = t * (float)GTE_C_PI;
        ctrl[0] = std::cos(angle) * mModule.radius;
        ctrl[1] = mModule.GetCurrentY3() + std::sin(angle) * mModule.radius;
        mCableSpline->SetControl(i, ctrl);
    }

    // Set control points for wire from pulley midline to mass 2.
    imin = imax + 1;
    imax = numCtrls - 1;
    if (imin < imax)
    {
        mult = 1.0f / static_cast<float>(imax - imin);
        ctrl[0] = mModule.radius;
        for (i = imin; i <= imax; ++i)
        {
            t = mult * static_cast<float>(i - imin);
            ctrl[1] = (1.0f - t) * mModule.GetCurrentY3() + t * mModule.GetCurrentY2();
            mCableSpline->SetControl(i, ctrl);
        }
    }
    else
    {
        mCableSpline->SetControl(imin, ctrl);
    }

    // Update the tube surface.
    mCableSurface->Update();
    mEngine->Update(mCable->GetVertexBuffer());

    // Update the mass positions.
    mMass1->localTransform.SetTranslation(-mModule.radius, mModule.GetCurrentY1(), 0.0f);
    mMass2->localTransform.SetTranslation(mModule.radius, mModule.GetCurrentY2(), 0.0f);
}

void MassPulleySpringSystemWindow3::UpdateHelix()
{
    // The current span of the helix.
    float span = mModule.GetCurrentY3() - mModule.radius - 4.0f;

    int32_t const numCtrls = mHelixSpline->GetNumControls();
    float const radius = 2.0f;
    float const tmax = 14.0f;
    float yMult = span / tmax;
    float delta = tmax / static_cast<float>(numCtrls - 1);
    for (int32_t i = 0; i < numCtrls; ++i)
    {
        float t = delta * static_cast<float>(i);
        float angle = t * (float)GTE_C_TWO_PI;
        float cs = std::cos(angle);
        float sn = std::sin(angle);
        Vector3<float> ctrl{ radius * cs, -mModule.radius - 4.0f - yMult*t, radius * sn };
        mHelixSpline->SetControl(i, ctrl);
    }

    // Update the tube surface.
    mHelixSurface->Update();
    mEngine->Update(mHelix->GetVertexBuffer());
}

void MassPulleySpringSystemWindow3::PhysicsTick()
{
    mModule.Update();

    UpdatePulley();
    UpdateCable();
    UpdateHelix();
    mAssembly->Update();
    mPVWMatrices.Update();
}

void MassPulleySpringSystemWindow3::GraphicsTick()
{
    mEngine->ClearBuffers();
    for (auto const& visual : mVisuals)
    {
        mEngine->Draw(visual);
    }
    mEngine->DisplayColorBuffer(0);
}
