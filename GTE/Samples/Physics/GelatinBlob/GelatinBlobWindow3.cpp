// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "GelatinBlobWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>
#include <Graphics/Texture2Effect.h>
#include <Mathematics/EdgeKey.h>
#include <random>

GelatinBlobWindow3::GelatinBlobWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.713725f, 0.807843f, 0.929411f, 1.0f });

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;

    mDepthReadNoWriteState = std::make_shared<DepthStencilState>();
    mDepthReadNoWriteState->depthEnable = true;
    mDepthReadNoWriteState->writeMask = DepthStencilState::WriteMask::ZERO;

    mNoCullSolidState = std::make_shared<RasterizerState>();
    mNoCullSolidState->fill = RasterizerState::Fill::SOLID;
    mNoCullSolidState->cull = RasterizerState::Cull::NONE;
    mEngine->SetRasterizerState(mNoCullSolidState);

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mNoCullWireState->cull = RasterizerState::Cull::NONE;

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.01f,
        { 0.0f, -5.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
}

void GelatinBlobWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#if !defined(GELATIN_BLOB_SINGLE_STEP)
    PhysicsTick();
#endif
    GraphicsTick();

    mTimer.UpdateFrameCount();
}

bool GelatinBlobWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mNoCullSolidState == mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullSolidState);
        }
        return true;

#if defined(GELATIN_BLOB_SINGLE_STEP)
    case 'g':
    case 'G':
        PhysicsTick();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool GelatinBlobWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Water.png") == "")
    {
        LogError("Cannot find file Water.png");
        return false;
    }

    return true;
}

void GelatinBlobWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();
    CreateIcosahedron();
    CreateSprings();
    CreateSegments();
    mTrackBall.Attach(mScene);
    mTrackBall.Update();
}

void GelatinBlobWindow3::CreateIcosahedron()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mf.SetVertexBufferUsage(Resource::Usage::DYNAMIC_UPDATE);
    mIcosahedron = mf.CreateIcosahedron();

    // Load the water texture and modify the alpha channel to 0.5 for some
    // transparency.
    auto texture = WICFileIO::Load(mEnvironment.GetPath("Water.png"), false);
    uint32_t numTexels = texture->GetNumElements();
    auto texels = texture->Get<uint32_t>();
    for (uint32_t i = 0; i < numTexels; ++i)
    {
        texels[i] = (texels[i] & 0x00FFFFFF) | 0x80000000;
    }

    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);
    mIcosahedron->SetEffect(effect);
    mPVWMatrices.Subscribe(mIcosahedron->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mIcosahedron);
}

void GelatinBlobWindow3::CreateSprings()
{
    // The icosahedron has 12 vertices and 30 edges.  Each vertex is a
    // particle in the system.  Each edge represents a spring.  To keep the
    // icosahedron from collapsing, 12 immovable particles are added, each
    // outside the icosahedron in the normal direction above a vertex.  The
    // immovable particles are connected to their corresponding vertices with
    // springs.
    int32_t const numParticles = 24, numSprings = 42;

    // Viscous forces applied.  If you set viscosity to zero, the cuboid
    // wiggles indefinitely since there is no dissipation of energy.  If
    // the viscosity is set to a positive value, the oscillations eventually
    // stop.  The length of time to steady state is inversely proportional
    // to the viscosity.
    float const step = 0.001f;
    float const viscosity = 0.01f;
    mModule = std::make_unique<PhysicsModule>(numParticles, numSprings, step, viscosity);

    // Set positions and velocities.  The first 12 positions are the vertices
    // of the icosahedron.  The last 12 are the extra particles added to
    // stabilize the system.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-0.1f, 0.1f);
    float constexpr fmax = std::numeric_limits<float>::max();
    auto const& vbuffer = mIcosahedron->GetVertexBuffer();
    Vertex* vertices = vbuffer->Get<Vertex>();
    for (int32_t i = 0; i < 12; ++i)
    {
        mModule->SetMass(i, 1.0f);
        mModule->SetPosition(i, vertices[i].position);
        mModule->SetVelocity(i, { rnd(mte), rnd(mte), rnd(mte) });
    }
    for (int32_t i = 12; i < 24; ++i)
    {
        mModule->SetMass(i, fmax);
        mModule->SetPosition(i, 2.0f * vertices[i - 12].position);
        mModule->SetVelocity(i, { 0.0f, 0.0f, 0.0f });
    }

    // Get unique set of edges for icosahedron.
    std::set<EdgeKey<false>> edgeSet;
    auto const& ibuffer = mIcosahedron->GetIndexBuffer();
    uint32_t const numTriangles = ibuffer->GetNumPrimitives();
    auto indices = ibuffer->Get<uint32_t>();
    for (uint32_t t = 0; t < numTriangles; ++t)
    {
        int32_t v0 = *indices++;
        int32_t v1 = *indices++;
        int32_t v2 = *indices++;
        edgeSet.insert(EdgeKey<false>(v0, v1));
        edgeSet.insert(EdgeKey<false>(v1, v2));
        edgeSet.insert(EdgeKey<false>(v2, v0));
    }

    // Springs are at rest in the initial configuration.
    float const constant = 10.0f;
    int32_t index = 0;
    MassSpringArbitrary<3, float>::Spring spring;
    for (auto const& edge : edgeSet)
    {
        spring.particle0 = edge.V[0];
        spring.particle1 = edge.V[1];
        spring.constant = constant;
        spring.length = Length(mModule->GetPosition(spring.particle1)
            - mModule->GetPosition(spring.particle0));

        mModule->SetSpring(index, spring);
        ++index;
    }
    for (int32_t i = 0; i < 12; ++i)
    {
        spring.particle0 = i;
        spring.particle1 = i + 12;
        spring.constant = constant;
        spring.length = Length(mModule->GetPosition(spring.particle1)
            - mModule->GetPosition(spring.particle0));

        mModule->SetSpring(index, spring);
        ++index;
    }
}

void GelatinBlobWindow3::CreateSegments()
{
    mSegmentRoot = std::make_shared<Node>();
    mScene->AttachChild(mSegmentRoot);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);
    Vector4<float> white{ 1.0f, 1.0f, 1.0f, 1.0f };

    int32_t const numSprings = mModule->GetNumSprings();
    for (int32_t index = 0; index < numSprings; ++index)
    {
        MassSpringArbitrary<3, float>::Spring spring = mModule->GetSpring(index);

        auto vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
        vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
        auto positions = vbuffer->Get<Vector3<float>>();
        positions[0] = mModule->GetPosition(spring.particle0);
        positions[1] = mModule->GetPosition(spring.particle1);

        auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, white);

        auto segment = std::make_shared<Visual>(vbuffer, ibuffer, effect);
        mPVWMatrices.Subscribe(segment->worldTransform, effect->GetPVWMatrixConstant());
        mSegmentRoot->AttachChild(segment);
        mSegments.push_back(segment);
    }
}

void GelatinBlobWindow3::PhysicsTick()
{
    mModule->Update(static_cast<float>(mMotionTimer.GetSeconds()));

    // Update icosahedron.  The particle system and icosahedron maintain
    // their own copy of the vertices, so this update is necessary.
    std::shared_ptr<VertexBuffer> vbuffer = mIcosahedron->GetVertexBuffer();
    auto vertices = vbuffer->Get<Vertex>();
    for (int32_t i = 0; i < 12; ++i)
    {
        vertices[i].position = mModule->GetPosition(i);
    }
    mEngine->Update(vbuffer);

    // Update the segments representing the springs.
    int32_t const numSprings = mModule->GetNumSprings();
    for (int32_t index = 0; index < numSprings; ++index)
    {
        MassSpringArbitrary<3, float>::Spring spring = mModule->GetSpring(index);

        auto segment = std::static_pointer_cast<Visual>(mSegmentRoot->GetChild(index));
        vbuffer = segment->GetVertexBuffer();
        auto positions = vbuffer->Get<Vector3<float>>();
        positions[0] = mModule->GetPosition(spring.particle0);
        positions[1] = mModule->GetPosition(spring.particle1);
        mEngine->Update(vbuffer);
    }
}

void GelatinBlobWindow3::GraphicsTick()
{
    mEngine->ClearBuffers();

    for (auto const& segment : mSegments)
    {
        mEngine->Draw(segment);
    }

    auto const& previousBlendState = mEngine->GetBlendState();
    mEngine->SetBlendState(mBlendState);
    mEngine->SetDepthStencilState(mDepthReadNoWriteState);
    mEngine->Draw(mIcosahedron);
    mEngine->SetDefaultDepthStencilState();
    mEngine->SetBlendState(previousBlendState);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);
}
