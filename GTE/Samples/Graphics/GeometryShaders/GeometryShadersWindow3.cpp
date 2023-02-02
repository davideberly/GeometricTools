// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "GeometryShadersWindow3.h"
#include <Applications/WICFileIO.h>
#include <random>

GeometryShadersWindow3::GeometryShadersWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 2.8f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

#if defined(SAVE_RENDERING_TO_DISK)
    mTarget = std::make_shared<DrawTarget>(1, DF_R8G8B8A8_UNORM, mXSize, mYSize);
    mTarget->GetRTTexture(0)->SetCopy(Resource::Copy::STAGING_TO_CPU);
#endif
}

void GeometryShadersWindow3::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

#if defined(SAVE_RENDERING_TO_DISK)
    mEngine->Enable(mTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Disable(mTarget);
    mEngine->CopyGpuToCpu(mTarget->GetRTTexture(0));
    WICFileIO::SaveToPNG("GeometryShaders.png", mTarget->GetRTTexture(0));
#endif

    mTimer.UpdateFrameCount();
}

bool GeometryShadersWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/GeometryShaders/Shaders/");
    std::vector<std::string> inputs =
    {
        mEngine->GetShaderName("RandomSquaresDirect.vs"),
        mEngine->GetShaderName("RandomSquaresDirect.gs"),
        mEngine->GetShaderName("RandomSquaresDirect.ps"),
        mEngine->GetShaderName("RandomSquaresIndirect.vs"),
        mEngine->GetShaderName("RandomSquaresIndirect.gs"),
        mEngine->GetShaderName("RandomSquaresIndirect.ps")
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

bool GeometryShadersWindow3::CreateScene()
{
    std::string vsPath, gsPath, psPath;
#if defined(USE_DRAW_DIRECT)
    vsPath = mEnvironment.GetPath(mEngine->GetShaderName("RandomSquaresDirect.vs"));
    gsPath = mEnvironment.GetPath(mEngine->GetShaderName("RandomSquaresDirect.gs"));
    psPath = mEnvironment.GetPath(mEngine->GetShaderName("RandomSquaresDirect.ps"));
#else
    vsPath = mEnvironment.GetPath(mEngine->GetShaderName("RandomSquaresIndirect.vs"));
    gsPath = mEnvironment.GetPath(mEngine->GetShaderName("RandomSquaresIndirect.gs"));
    psPath = mEnvironment.GetPath(mEngine->GetShaderName("RandomSquaresIndirect.ps"));
#endif
    auto program = mProgramFactory->CreateFromFiles(vsPath, psPath, gsPath);
    if (!program)
    {
        return false;
    }

    // Create particles used by direct and indirect drawing.
    struct Vertex
    {
        Vector4<float> position;
        Vector4<float> colorSize;
    };

    // Use a Mersenne twister engine for random numbers.
    std::mt19937 mte;
    std::uniform_real_distribution<float> symr(-1.0f, 1.0f);
    std::uniform_real_distribution<float> unir(0.0f, 1.0f);
    std::uniform_real_distribution<float> posr(0.01f, 0.1f);

    int32_t const numParticles = 128;
    std::vector<Vertex> particles(numParticles);
    for (auto& particle : particles)
    {
        particle.position = { symr(mte), symr(mte), symr(mte), 1.0f };
        particle.colorSize = { unir(mte), unir(mte), unir(mte), posr(mte) };
    }

    // Create the constant buffer used by direct and indirect drawing.
    mMatrices = std::make_shared<ConstantBuffer>(2 * sizeof(Matrix4x4<float>), true);
    program->GetGeometryShader()->Set("Matrices", mMatrices);

#if defined(USE_DRAW_DIRECT)
    // Create a mesh for direct drawing.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32A32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numParticles);
    std::memcpy(vbuffer->GetData(), &particles[0], numParticles * sizeof(Vertex));
#else
    // Create a mesh for indirect drawing.
    auto vbuffer = std::make_shared<VertexBuffer>(numParticles);
    mParticles = std::make_shared<StructuredBuffer>(numParticles, sizeof(Vertex));
    std::memcpy(mParticles->GetData(), &particles[0], numParticles * sizeof(Vertex));
    program->GetGeometryShader()->Set("particles", mParticles);
#endif

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYPOINT, numParticles);
    auto effect = std::make_shared<VisualEffect>(program);
    mMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    return true;
}

void GeometryShadersWindow3::UpdateConstants()
{
    Matrix4x4<float> wMatrix = mTrackBall.GetOrientation();
    Matrix4x4<float> vMatrix = mCamera->GetViewMatrix();
    Matrix4x4<float> pMatrix = mCamera->GetProjectionMatrix();
    Matrix4x4<float> vwMatrix = DoTransform(vMatrix, wMatrix);

    mMatrices->SetMember("vwMatrix", vwMatrix);
    mMatrices->SetMember("pMatrix", pMatrix);
    mEngine->Update(mMatrices);
}
