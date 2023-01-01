// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BufferUpdatingWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>

#define TEST_UPDATE
//#define TEST_COPY_CPU_TO_GPU
//#define TEST_COPY_GPU_TO_CPU

BufferUpdatingWindow3::BufferUpdatingWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    // Create a flat surface with a gridded texture.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mSurface = mf.CreateRectangle(NUM_SAMPLES, NUM_SAMPLES, 1.0f, 1.0f);
    auto const& vbuffer = mSurface->GetVertexBuffer();

#if defined(TEST_UPDATE)
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
#endif
#if defined(TEST_COPY_CPU_TO_GPU)
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    vbuffer->SetCopy(Resource::Copy::CPU_TO_STAGING);
#endif
#if defined(TEST_COPY_GPU_TO_CPU)
    // Start with flat height field, offset the middle row on CPU and
    // copy to GPU, copy from GPU to CPU, modify the middle row, copy
    // from CPU to GPU.  Thus, we need the COPY_BIDIRECTIONAL flag.  If
    // all you do is copy from GPU to CPU, then use COPY_STAGING_TO_CPU.
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    vbuffer->SetCopy(Resource::Copy::BIDIRECTIONAL);
#endif
    mEngine->Bind(vbuffer);

    mEnvironment.Insert(GetGTEPath() + "/Samples/Data/");
    std::string path = mEnvironment.GetPath("BlueGrid.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory,
        texture, SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::CLAMP,
        SamplerState::Mode::CLAMP);
    mSurface->SetEffect(effect);
    mPVWMatrices.Subscribe(mSurface->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mSurface);

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0, 0.001f, 0.001f,
        { 0.0f, 0.0f, 4.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();
}

void BufferUpdatingWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    // Offset the middle row of vertices of the flat surface.
    auto const& vbuffer = mSurface->GetVertexBuffer();
    uint32_t saveOffset = vbuffer->GetOffset();
    uint32_t saveNumActiveElements = vbuffer->GetNumActiveElements();
    vbuffer->SetNumActiveElements(NUM_SAMPLES);
    vbuffer->SetOffset(NUM_SAMPLES * NUM_SAMPLES / 2);
    Vertex* vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < vbuffer->GetNumActiveElements(); ++i)
    {
        vertices[i + vbuffer->GetOffset()].position[2] = 1.0f;
    }

    // All frame rates are reported for an NVIDIA GeForce GTX 1080.
#if defined(TEST_UPDATE)
    // 280 fps (DX11), 1770 fps (OpenGL)
    mEngine->Update(vbuffer);
#endif
#if defined(TEST_COPY_CPU_TO_GPU)
    // 270 fps (DX11), 1810 fps (OpenGL)
    mEngine->CopyCpuToGpu(vbuffer);
#endif
#if defined(TEST_COPY_GPU_TO_CPU)
    // 260 fps (DX11), 1750 fps (OpenGL)
    mEngine->CopyCpuToGpu(vbuffer);
    mEngine->CopyGpuToCpu(vbuffer);
    float invNumElements = 1.0f / static_cast<float>(vbuffer->GetNumActiveElements());
    for (uint32_t i = 0; i < vbuffer->GetNumActiveElements(); ++i)
    {
        vertices[i + vbuffer->GetOffset()].position[2] -= i * invNumElements;
    }
    mEngine->CopyCpuToGpu(vbuffer);
#endif
    vbuffer->SetOffset(saveOffset);
    vbuffer->SetNumActiveElements(saveNumActiveElements);

    mEngine->ClearBuffers();
    mEngine->Draw(mSurface);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool BufferUpdatingWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
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
    }
    return Window3::OnCharPress(key, x, y);
}
