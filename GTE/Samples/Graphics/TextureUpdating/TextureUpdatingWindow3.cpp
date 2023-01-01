// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "TextureUpdatingWindow3.h"
#include <Graphics/Texture2Effect.h>

TextureUpdatingWindow3::TextureUpdatingWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mForward(true)
{
    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0, 0.001f, 0.001f,
        { 0.0f, 0.0f, 1.25 }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();
}

void TextureUpdatingWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mSquare);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();

    // Access the current texture for the effect.
    auto data = reinterpret_cast<uint8_t*>(mTexture->GetData());
    uint32_t const rowBytes = mTexture->GetWidth() * mTexture->GetElementSize();

    // First clear all the values in the CPU copy of the texture.
    memset(data, 0, static_cast<size_t>(rowBytes) * static_cast<size_t>(mTexture->GetHeight()));

    // Read the values back from the GPU.
    mEngine->CopyGpuToCpu(mTexture);

    // Move the rows "down" in a circular fashion.
    // Use the Update call on the texture.
    if (mForward)
    {
        uint32_t heightM1 = mTexture->GetHeight() - 1;
        data += static_cast<size_t>(rowBytes) * static_cast<size_t>(heightM1);
        std::vector<uint8_t*> saveRow(rowBytes);
        std::memcpy(saveRow.data(), data, rowBytes);
        for (uint32_t y = heightM1; y > 0; --y)
        {
            memmove(data, data-rowBytes, rowBytes);
            data -= rowBytes;
        }
        std::memcpy(data, saveRow.data(), rowBytes);
#if defined(GTE_USE_OPENGL)
        mEngine->Update(mTexture);
#else
        // TODO: DX11 automipmapped textures fail the Update(...) call,
        // probably due to being tagged as render targets.  Can this be
        // fixed?  Verify that in fact the automipmapped textures must
        // be render targets.
        mEngine->CopyCpuToGpu(mTexture);
#endif
    }

    // Move the rows "up" in a circular fashion.
    // Use the CopyCpuToGpu call on the texture.
    else
    {
        std::vector<uint8_t*> saveRow(rowBytes);
        std::memcpy(saveRow.data(), data, rowBytes);
        for (uint32_t y = 1; y < mTexture->GetHeight(); ++y)
        {
            memmove(data, data+rowBytes, rowBytes);
            data += rowBytes;
        }
        std::memcpy(data, saveRow.data(), rowBytes);
        mEngine->CopyCpuToGpu(mTexture);
    }
}

bool TextureUpdatingWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'f':
    case 'F':
        mForward = true;
        return true;

    case 'b':
    case 'B':
        mForward = false;
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void TextureUpdatingWindow3::CreateScene()
{
    // Create a vertex buffer for a two-triangles square.  The PNG is stored
    // in left-handed coordinates.  The texture coordinates are chosen to
    // reflect the texture in the y-direction.
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
    mTexture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, 256, 256, true, true);
    mTexture->AutogenerateMipmaps();
    mTexture->SetCopy(Resource::Copy::BIDIRECTIONAL);
    mTexture->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto* data = mTexture->Get<uint8_t>();
    for (uint32_t y = 0; y < mTexture->GetHeight(); ++y)
    {
        uint32_t const rowBytes = mTexture->GetWidth() * mTexture->GetElementSize();
        memset(data, y, rowBytes);
        data += rowBytes;
    }

    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, mTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);

    // Create the geometric object for drawing.  Translate it so that its
    // center of mass is at the origin.  This supports virtual trackball
    // motion about the object "center".
    mSquare = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSquare->localTransform.SetTranslation(-0.5f, -0.5f, 0.0f);

    // Enable automatic updates of pvw-matrices and w-matrices.
    mPVWMatrices.Subscribe(mSquare->worldTransform, effect->GetPVWMatrixConstant());

    mTrackBall.Attach(mSquare);
    mTrackBall.Update();
}
