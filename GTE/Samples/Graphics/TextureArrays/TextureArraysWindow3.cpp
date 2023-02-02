// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "TextureArraysWindow3.h"
#include <Applications/WICFileIO.h>

TextureArraysWindow3::TextureArraysWindow3(Parameters& parameters)
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

void TextureArraysWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mSquare);
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool TextureArraysWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/Graphics/TextureArrays/Shaders/");
    std::vector<std::string> inputs =
    {
        mEngine->GetShaderName("TextureArrays.vs"),
        mEngine->GetShaderName("TextureArrays.ps"),
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

bool TextureArraysWindow3::CreateScene()
{
    std::shared_ptr<VisualProgram> program;

    // Load and compile the shaders.
    std::string vsPath = mEnvironment.GetPath(mEngine->GetShaderName("TextureArrays.vs"));
    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("TextureArrays.ps"));
    program = mProgramFactory->CreateFromFiles(vsPath, psPath, "");
    if (!program)
    {
        return false;
    }

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

    // Create an effect for the vertex and pixel shaders.  The texture is
    // bilinearly filtered and the texture coordinates are clamped to [0,1]^2.
    auto cbuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVertexShader()->Set("PVWMatrix", cbuffer);

    auto const& pshader = program->GetPixelShader();
    auto t1array = std::make_shared<Texture1Array>(2, DF_R8G8B8A8_UNORM, 2);
    auto* t1data = t1array->Get<uint32_t>();
    t1data[0] = 0xFF000000;
    t1data[1] = 0xFFFFFFFF;

    auto stoneTexture = WICFileIO::Load(mEnvironment.GetPath("StoneWall.png"), false);
    auto t2array = std::make_shared<Texture2Array>(2, DF_R8G8B8A8_UNORM, 256, 256);
    uint8_t* t2data = t2array->Get<uint8_t>();
    size_t const numBytes = stoneTexture->GetNumBytes();
    std::memcpy(t2data, stoneTexture->GetData(), numBytes);
    t2data += numBytes;
    for (size_t i = 0; i < numBytes; ++i)
    {
        *t2data++ = static_cast<uint8_t>(rand() % 256);
    }

    auto samplerState = std::make_shared<SamplerState>();
    samplerState->filter = SamplerState::Filter::MIN_L_MAG_L_MIP_P;
    samplerState->mode[0] = SamplerState::Mode::CLAMP;
    samplerState->mode[1] = SamplerState::Mode::CLAMP;

    pshader->Set("myTexture1", t1array, "mySampler1", samplerState);
    pshader->Set("myTexture2", t2array, "mySampler2", samplerState);

    auto effect = std::make_shared<VisualEffect>(program);

    // Create the geometric object for drawing.  Translate it so that its
    // center of mass is at the origin.  This supports virtual trackball
    // motion about the object "center".
    mSquare = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSquare->localTransform.SetTranslation(-0.5f, -0.5f, 0.0f);

    // Enable automatic updates of pvw-matrices and w-matrices.
    mPVWMatrices.Subscribe(mSquare->worldTransform, cbuffer);

    mTrackBall.Attach(mSquare);
    mTrackBall.Update();
    return true;
}
