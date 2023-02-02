// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "StructuredBuffersWindow3.h"
#include <Applications/WICFileIO.h>

StructuredBuffersWindow3::StructuredBuffersWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.001f, 0.001f,
        { 0.0f, 0.0f, 1.25f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();
}

void StructuredBuffersWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    std::memset(mDrawnPixels->GetData(), 0, mDrawnPixels->GetNumBytes());
    mEngine->CopyCpuToGpu(mDrawnPixels);

    mEngine->ClearBuffers();
    mEngine->Draw(mSquare);

    mEngine->CopyGpuToCpu(mDrawnPixels);
    Vector4<float>* src = mDrawnPixels->Get<Vector4<float>>();
    uint32_t* trg = mDrawnPixelsTexture->Get<uint32_t>();
    for (int32_t i = 0; i < mXSize*mYSize; ++i)
    {
        uint32_t r = static_cast<uint8_t>(255.0f*src[i][0]);
        uint32_t g = static_cast<uint8_t>(255.0f*src[i][1]);
        uint32_t b = static_cast<uint8_t>(255.0f*src[i][2]);
        trg[i] = r | (g << 8) | (b << 16) | (0xFF << 24);
    }
    WICFileIO::SaveToPNG("DrawnPixels.png", mDrawnPixelsTexture);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool StructuredBuffersWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/Graphics/StructuredBuffers/Shaders/");
    std::vector<std::string> inputs =
    {
        mEngine->GetShaderName("StructuredBuffers.vs"),
        mEngine->GetShaderName("StructuredBuffers.ps"),
        "StoneWall.png"
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

bool StructuredBuffersWindow3::CreateScene()
{
    // Create the shaders and associated resources.
    std::string vsPath = mEnvironment.GetPath(mEngine->GetShaderName("StructuredBuffers.vs"));
    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("StructuredBuffers.ps"));
    mProgramFactory->defines.Set("WINDOW_WIDTH", mXSize);
    auto program = mProgramFactory->CreateFromFiles(vsPath, psPath, "");
    if (!program)
    {
        return false;
    }
    mProgramFactory->defines.Clear();

    auto cbuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVertexShader()->Set("PVWMatrix", cbuffer);

    // Create the pixel shader and associated resources.
    auto const& pshader = program->GetPixelShader();
    std::string path = mEnvironment.GetPath("StoneWall.png");
    auto baseTexture = WICFileIO::Load(path, false);
    auto baseSampler = std::make_shared<SamplerState>();
    baseSampler->filter = SamplerState::Filter::MIN_L_MAG_L_MIP_P;
    baseSampler->mode[0] = SamplerState::Mode::CLAMP;
    baseSampler->mode[1] = SamplerState::Mode::CLAMP;
    pshader->Set("baseTexture", baseTexture, "baseSampler", baseSampler);

    mDrawnPixels = std::make_shared<StructuredBuffer>(mXSize * mYSize, sizeof(Vector4<float>));
    mDrawnPixels->SetUsage(Resource::Usage::SHADER_OUTPUT);
    mDrawnPixels->SetCopy(Resource::Copy::BIDIRECTIONAL);
    std::memset(mDrawnPixels->GetData(), 0, mDrawnPixels->GetNumBytes());
    pshader->Set("drawnPixels", mDrawnPixels);

    // Create the visual effect for the square.
    auto effect = std::make_shared<VisualEffect>(program);

    // Create a vertex buffer for a single triangle.  The PNG is stored in
    // left-handed coordinates.  The texture coordinates are chosen to reflect
    // the texture in the y-direction.
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    auto* vertices = vbuffer->Get<Vertex>();
    vertices[0].position = { 0.0f, 0.0f, 0.0f };
    vertices[0].tcoord = { 0.0f, 1.0f };
    vertices[1].position = { 1.0f, 0.0f, 0.0f };
    vertices[1].tcoord = { 1.0f, 1.0f };
    vertices[2].position = { 0.0f, 1.0f, 0.0f };
    vertices[2].tcoord = { 0.0f, 0.0f };
    vertices[3].position = { 1.0f, 1.0f, 0.0f };
    vertices[3].tcoord = { 1.0f, 0.0f };

    // Create an indexless buffer for a triangle mesh with two triangles.
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRISTRIP, 2);

    // Create the geometric object for drawing.  Translate it so that its
    // center of mass is at the origin.  This supports virtual trackball
    // motion about the object "center".
    mSquare = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSquare->localTransform.SetTranslation(-0.5f, -0.5f, 0.0f);

    // Enable automatic updates of pvw-matrices and w-matrices.
    mPVWMatrices.Subscribe(mSquare->worldTransform, cbuffer);

    // The structured buffer is written in the pixel shader.  This texture
    // will receive a copy of it so that we can write the results to disk
    // as a PNG file.
    mDrawnPixelsTexture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, mXSize, mYSize);

    mTrackBall.Attach(mSquare);
    mTrackBall.Update();
    return true;
}
