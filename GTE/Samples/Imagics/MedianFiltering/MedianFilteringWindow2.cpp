// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "MedianFilteringWindow2.h"
#include <random>

MedianFilteringWindow2::MedianFilteringWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mSelection(0)
{
    uint32_t const txWidth = 1024, txHeight = 1024;
    if (!SetEnvironment() || !CreatePrograms(txWidth, txHeight))
    {
        parameters.created = false;
        return;
    }

    mOriginal = std::make_shared<Texture2>(DF_R32_FLOAT, txWidth, txHeight);
    for (int32_t i = 0; i < 2; ++i)
    {
        mImage[i] = std::make_shared<Texture2>(DF_R32_FLOAT, txWidth, txHeight);
        mImage[i]->SetUsage(Resource::Usage::SHADER_OUTPUT);
        mImage[i]->SetCopy(Resource::Copy::BIDIRECTIONAL);
    }

    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.0625f, 1.0f);
    auto* data = mOriginal->Get<float>();
    for (uint32_t i = 0; i < txWidth * txHeight; ++i)
    {
        data[i] = rnd(mte);
    }
    std::memcpy(mImage[0]->GetData(), data, mImage[0]->GetNumBytes());
    std::memcpy(mImage[1]->GetData(), data, mImage[1]->GetNumBytes());

    // Create two overlays, one for the original image and one for the
    // median-filtered image.
    std::array<int32_t, 4> rect[2] =
    {
        { 0, 0, mXSize / 2, mYSize },
        { mXSize / 2, 0, mXSize / 2, mYSize }
    };
    for (int32_t i = 0; i < 2; ++i)
    {
        mOverlay[i] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize, mYSize, txWidth, txHeight,
            SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP, false);
        mOverlay[i]->SetOverlayRectangle(rect[i]);
    }
    mOverlay[0]->SetTexture(mOriginal);
    mOverlay[1]->SetTexture(mImage[1]);

    for (int32_t i = 0; i < 4; ++i)
    {
        auto const& cshader = mMedianProgram[i]->GetComputeShader();
        cshader->Set("inImage", mImage[0]);
        cshader->Set("outImage", mImage[1]);
    }
    mCProgram = mMedianProgram[0];
}

void MedianFilteringWindow2::OnIdle()
{
    mTimer.Measure();

    mEngine->Execute(mCProgram, mNumXGroups, mNumYGroups, 1);
    mEngine->Draw(mOverlay[0]);
    mEngine->Draw(mOverlay[1]);
    std::swap(mImage[0], mImage[1]);
    auto const& cshader = mCProgram->GetComputeShader();
    cshader->Set("inImage", mImage[0]);
    cshader->Set("outImage", mImage[1]);
    mOverlay[1]->SetTexture(mImage[1]);
    std::array<float, 4> textColor{ 1.0f, 1.0f, 0.0f, 1.0f };
    mEngine->Draw(8, mYSize - 24, textColor, msName[mSelection]);
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool MedianFilteringWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case '0':
        mSelection = 0;
        std::memcpy(mImage[0]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[0]);
        std::memcpy(mImage[1]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[1]);
        mCProgram = mMedianProgram[0];
        return true;

    case '1':
        mSelection = 1;
        std::memcpy(mImage[0]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[0]);
        std::memcpy(mImage[1]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[1]);
        mCProgram = mMedianProgram[1];
        return true;

    case '2':
        mSelection = 2;
        std::memcpy(mImage[0]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[0]);
        std::memcpy(mImage[1]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[1]);
        mCProgram = mMedianProgram[2];
        return true;

    case '3':
        mSelection = 3;
        std::memcpy(mImage[0]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[0]);
        std::memcpy(mImage[1]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[1]);
        mCProgram = mMedianProgram[3];
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool MedianFilteringWindow2::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Imagics/MedianFiltering/Shaders/");
    std::vector<std::string> inputs =
    {
        mEngine->GetShaderName("Median3x3.cs"),
        mEngine->GetShaderName("Median5x5.cs"),
        mEngine->GetShaderName("MedianBySort.cs")
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

bool MedianFilteringWindow2::CreatePrograms(uint32_t txWidth, uint32_t txHeight)
{
    // Create the shaders.
    int32_t const numThreads = 8;
    mNumXGroups = txWidth / numThreads;
    mNumYGroups = txHeight / numThreads;

    mProgramFactory->defines.Set("NUM_X_THREADS", numThreads);
    mProgramFactory->defines.Set("NUM_Y_THREADS", numThreads);

    std::string csPath = mEnvironment.GetPath(mEngine->GetShaderName("MedianBySort.cs"));
    mProgramFactory->defines.Set("RADIUS", 1);
    mMedianProgram[0] = mProgramFactory->CreateFromFile(csPath);
    if (!mMedianProgram[0])
    {
        return false;
    }

    csPath = mEnvironment.GetPath(mEngine->GetShaderName("Median3x3.cs"));
    mMedianProgram[1] = mProgramFactory->CreateFromFile(csPath);
    if (!mMedianProgram[1])
    {
        return false;
    }

    mProgramFactory->defines.Set("RADIUS", 2);
    csPath = mEnvironment.GetPath(mEngine->GetShaderName("MedianBySort.cs"));
    mMedianProgram[2] = mProgramFactory->CreateFromFile(csPath);
    if (!mMedianProgram[2])
    {
        return false;
    }

    csPath = mEnvironment.GetPath(mEngine->GetShaderName("Median5x5.cs"));
    mMedianProgram[3] = mProgramFactory->CreateFromFile(csPath);
    if (!mMedianProgram[3])
    {
        return false;
    }

    mProgramFactory->defines.Clear();
    return true;
}

std::string MedianFilteringWindow2::msName[4] =
{
    "median 3x3 by insertion sort",
    "median 3x3 by min-max",
    "median 5x5 by insertion sort",
    "median 5x5 by min-max"
};
