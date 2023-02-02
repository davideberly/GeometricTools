// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "Fluids3DWindow3.h"
#include <Graphics/MeshFactory.h>

Fluids3DWindow3::Fluids3DWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mFluid(mEngine, mProgramFactory, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0.002f)
{
    if (!SetEnvironment() || !CreateNestedBoxes())
    {
        parameters.created = false;
        return;
    }

    // Use blending for the visualization.
    mAlphaState = std::make_shared<BlendState>();
    mAlphaState->target[0].enable = true;
    mAlphaState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mAlphaState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mAlphaState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mAlphaState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;
    mEngine->SetBlendState(mAlphaState);

    // The alpha channel must be zero for the blending of density to work
    // correctly through the fluid region.
    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, 0.0f });

    // The geometric proxies for volume rendering are concentric boxes.  They
    // are drawn from inside to outside for correctly sorted drawing, so depth
    // buffering is not needed.
    mNoDepthState = std::make_shared<DepthStencilState>();
    mNoDepthState->depthEnable = false;
    mEngine->SetDepthStencilState(mNoDepthState);

    mFluid.Initialize();
    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.01f, 0.001f,
        { 0.0f, 0.0f, -2.25f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();

    UpdateConstants();
}

void Fluids3DWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }
    UpdateConstants();

    mFluid.DoSimulationStep();

    mEngine->ClearBuffers();
    for (auto visual : mVisible)
    {
        mEngine->Draw(visual);
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(1);

    mTimer.UpdateFrameCount();
}

bool Fluids3DWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    if (key == '0')
    {
        mFluid.Initialize();
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool Fluids3DWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Physics/Fluids3D/Shaders/");

    if (mEnvironment.GetPath(mEngine->GetShaderName("VolumeRender.vs")) == "")
    {
        LogError("Cannot find file " + mEngine->GetShaderName("VolumeRender.vs"));
        return false;
    }
    if (mEnvironment.GetPath(mEngine->GetShaderName("VolumeRender.ps")) == "")
    {
        LogError("Cannot find file " + mEngine->GetShaderName("VolumeRender.ps"));
        return false;
    }

    return true;
}

bool Fluids3DWindow3::CreateNestedBoxes()
{
    std::string vsPath = mEnvironment.GetPath(mEngine->GetShaderName("VolumeRender.vs"));
    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("VolumeRender.ps"));
    std::shared_ptr<VisualProgram> program = mProgramFactory->CreateFromFiles(vsPath, psPath, "");
    if (!program)
    {
        return false;
    }

    mPVWMatrixBuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVertexShader()->Set("PVWMatrix", mPVWMatrixBuffer);
    mPVWMatrixBuffer->SetMember("pvwMatrix", Matrix4x4<float>::Identity());

    auto volumeSampler = std::make_shared<SamplerState>();
    volumeSampler->filter = SamplerState::Filter::MIN_L_MAG_L_MIP_P;
    volumeSampler->mode[0] = SamplerState::Mode::CLAMP;
    volumeSampler->mode[1] = SamplerState::Mode::CLAMP;
    volumeSampler->mode[2] = SamplerState::Mode::CLAMP;

    auto const& pshader = program->GetPixelShader();
    pshader->Set("volumeTexture", mFluid.GetState(), "volumeSampler", volumeSampler);

    auto effect = std::make_shared<VisualEffect>(program);

    struct Vertex
    {
        Vector3<float> position, tcoord;
    };
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32B32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int32_t const numBoxes = 128;
    float divisor = static_cast<float>(numBoxes - 1);
    for (int32_t i = 1; i <= numBoxes; ++i)
    {
        float extent = 0.5f * static_cast<float>(i) / divisor;
        auto visual(mf.CreateBox(extent, extent, extent));
        auto const& vbuffer = visual->GetVertexBuffer();
        auto vertex = vbuffer->Get<Vertex>();
        for (uint32_t j = 0; j < vbuffer->GetNumElements(); ++j, ++vertex)
        {
            Vector3<float>& tcd = vertex->tcoord;
            Vector3<float> pos = vertex->position;
            Vector4<float> tmp{ pos[0] + 0.5f, pos[1] + 0.5f, pos[2] + 0.5f, 0.0f };
            for (int32_t k = 0; k < 3; ++k)
            {
                tcd[k] = 0.5f * (tmp[k] + 1.0f);
            }
        }

        visual->SetEffect(effect);
        mVisible.push_back(visual);
    }

    return true;
}

void Fluids3DWindow3::UpdateConstants()
{
    Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
    Matrix4x4<float> wMatrix = mTrackBall.GetOrientation();
    Matrix4x4<float>& pvwMatrix = *mPVWMatrixBuffer->Get<Matrix4x4<float>>();
    pvwMatrix = DoTransform(pvMatrix, wMatrix);
    mEngine->Update(mPVWMatrixBuffer);
}
