// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BlownGlassWindow3.h"

BlownGlassWindow3::BlownGlassWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    // Use blending for the visualization.
    mMeshBlendState = std::make_shared<BlendState>();
    mMeshBlendState->target[0].enable = true;
    mMeshBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mMeshBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mMeshBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mMeshBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;

    // The alpha channel must be zero for the blending of density to work
    // correctly through the fluid region.
    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, 0.0f });

    // Disable face culling.
    mMeshRasterizerState = std::make_shared<RasterizerState>();
    mMeshRasterizerState->cull = RasterizerState::Cull::NONE;

    // Read the depth buffer but do not write to it.
    mMeshDepthStencilState = std::make_shared<DepthStencilState>();
    mMeshDepthStencilState->writeMask = DepthStencilState::WriteMask::ZERO;

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 2.5f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
}

void BlownGlassWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mFluid->DoSimulationStep();

    mEngine->ClearBuffers();

    mEngine->SetBlendState(mMeshBlendState);
    mEngine->SetRasterizerState(mMeshRasterizerState);
    mEngine->SetDepthStencilState(mMeshDepthStencilState);
    mEngine->Draw(mMesh);
    mEngine->SetDefaultDepthStencilState();
    mEngine->SetDefaultRasterizerState();
    mEngine->SetDefaultBlendState();

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(1);

    mTimer.UpdateFrameCount();
}

bool BlownGlassWindow3::SetEnvironment()
{
    // Set the search path to find images to load.
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }
    
    mEnvironment.Insert(path + "/Samples/Physics/BlownGlass/Data/");
    mEnvironment.Insert(path + "/Samples/Physics/BlownGlass/Shaders/");

    std::vector<std::string> inputs =
    {
        "Vertices82832.raw",
        "Indices41388.raw",
        mEngine->GetShaderName("VolumeRender.vs"),
        mEngine->GetShaderName("VolumeRender.ps")
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

bool BlownGlassWindow3::CreateScene()
{
    std::string path;

    // Create the shaders.
    std::string vsPath = mEnvironment.GetPath(mEngine->GetShaderName("VolumeRender.vs"));
    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("VolumeRender.ps"));
    auto program = mProgramFactory->CreateFromFiles(vsPath, psPath, "");
    if (!program)
    {
        return false;
    }

    // Create the fluid simulator.
    mFluid = std::make_unique<GPUFluid3>(mEngine, mProgramFactory, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0.002f);
    mFluid->Initialize();

    // Create the vertex shader for visualization.
    auto cbuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVertexShader()->Set("PVWMatrix", cbuffer);
    cbuffer->SetMember("pvwMatrix", Matrix4x4<float>::Identity());

    // Create the pixel shader for visualization.
    auto sampler = std::make_shared<SamplerState>();
    sampler->filter = SamplerState::Filter::MIN_L_MAG_L_MIP_P;
    sampler->mode[0] = SamplerState::Mode::CLAMP;
    sampler->mode[1] = SamplerState::Mode::CLAMP;
    sampler->mode[2] = SamplerState::Mode::CLAMP;
    program->GetPixelShader()->Set("volumeTexture", mFluid->GetState(), "volumeSampler", sampler);

    auto effect = std::make_shared<VisualEffect>(program);

    // Load the level-surface mesh obtained from the SurfaceExtraction sample.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    uint32_t numVertices = 82832;
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    path = mEnvironment.GetPath("Vertices82832.raw");
    std::ifstream input(path, std::ios::binary);
    input.read(vbuffer->GetData(), vbuffer->GetNumBytes());
    input.close();

    uint32_t numTriangles = 41388;
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(uint32_t));
    path = mEnvironment.GetPath("Indices41388.raw");
    input.open(path, std::ios::in | std::ios::binary);
    input.read(ibuffer->GetData(), ibuffer->GetNumBytes());
    input.close();

    mMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mMesh->localTransform.SetTranslation(-1.0f, -1.0f, -1.0f);

    // Automatic update of transforms for virtual trackball.
    mPVWMatrices.Subscribe(mMesh->worldTransform, cbuffer);

    mTrackBall.Attach(mMesh);
    mTrackBall.Update();
    return true;
}
