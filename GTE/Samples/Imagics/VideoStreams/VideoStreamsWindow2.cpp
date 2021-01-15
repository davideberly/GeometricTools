// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "VideoStreamsWindow2.h"
#include <random>

VideoStreamsWindow2::~VideoStreamsWindow2()
{
#if defined(DO_TRIGGERED_SERIAL) || defined(DO_TRIGGERED_PARALLEL)
    mVideoStreamManager->StopTriggeredCapture();
#endif
}

VideoStreamsWindow2::VideoStreamsWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mVideoStreams(NUM_VIDEO_STREAMS),
    mOverlay(NUM_VIDEO_STREAMS),
    mVideoStreamManager(nullptr),
    mCurrent(NUM_VIDEO_STREAMS)
{
    // Locate the context for the application regardless of where the
    // executable is launched.
    std::string path = GetGTEPath();
    if (path == "")
    {
        parameters.created = false;
        return;
    }
    path += "/Samples/Imagics/VideoStreams/Data/";
    mEnvironment.Insert(path);

    int const txWidth = 640, txHeight = 512;

    // Generate dummy video files.  This avoids having to post large data
    // files for the sample.  After you have created these the first time,
    // you can change the "#if 1" to "if 0" to reduce program initialization
    // time should you decide to experiment with the code.
    int const numImages = 16;
    int const format = static_cast<int>(DF_R8G8B8A8_UNORM);
    unsigned int colorMask[NUM_VIDEO_STREAMS] =
    {
        0xFF0000FF,
        0xFF00FF00,
        0xFFFF0000,
        0xFFFFFFFF
    };
    std::vector<unsigned int> texels(txWidth * txHeight);
    std::mt19937 mte;
    std::uniform_int_distribution<unsigned int> rnd(0, 127);
    for (int i = 0; i < NUM_VIDEO_STREAMS; ++i)
    {
        std::string name = "VideoStream" + std::to_string(i) + ".raw";
        if (mEnvironment.GetPath(name) != "")
        {
            continue;
        }
        name = path + name;
        std::ofstream output(name, std::ios::binary);
        if (!output)
        {
            parameters.created = false;
            return;
        }
        output.write((char const*)&numImages, sizeof(numImages));
        output.write((char const*)&format, sizeof(format));
        output.write((char const*)&txWidth, sizeof(txWidth));
        output.write((char const*)&txHeight, sizeof(txHeight));
        unsigned int mask = colorMask[i];
        for (int j = 0; j < numImages; ++j)
        {
            // Randomly generate an RGBA image.
            for (auto& texel : texels)
            {
                unsigned int r = 128 + rnd(mte);
                unsigned int g = 128 + rnd(mte);
                unsigned int b = 128 + rnd(mte);
                texel = mask & (r | (g << 8) | (b << 16) | 0xFF000000);
            }

            // Use index j as the frame number.
            output.write((char const*)&j, sizeof(j));
            output.write((char const*)&texels[0], 4 * texels.size());
        }
        output.close();
    }

    if (!CreateOverlays(txWidth, txHeight))
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    for (int i = 0; i < NUM_VIDEO_STREAMS; ++i)
    {
        std::string name = "VideoStream" + std::to_string(i) + ".raw";
        mVideoStreams[i] = std::make_shared<FileVideoStream>(
            mEnvironment.GetPath(name), mEngine);
    }
    mVideoStreamManager = std::make_unique<VideoStreamManager>(mVideoStreams, 30);

#if defined(DO_TRIGGERED_SERIAL)
    mVideoStreamManager->StartTriggeredCapture(30.0, false);
#endif

#if defined(DO_TRIGGERED_PARALLEL)
    mVideoStreamManager->StartTriggeredCapture(30.0, true);
#endif
}

void VideoStreamsWindow2::OnIdle()
{
#if defined(DO_MANUAL_SERIAL)
    mVideoStreamManager->CaptureFrameSerial();
    if (mVideoStreamManager->GetFrame(mCurrent))
    {
        for (int i = 0; i < 4; ++i)
        {
            mOverlay[i]->SetTexture(mCurrent.frames[i].image);
            mEngine->Draw(mOverlay[i]);
        }
        DrawStatistics();
        mEngine->DisplayColorBuffer(0);
    }
#endif

#if defined(DO_MANUAL_PARALLEL)
    mVideoStreamManager->CaptureFrameParallel();
    if (mVideoStreamManager->GetFrame(mCurrent))
    {
        for (int i = 0; i < 4; ++i)
        {
            mOverlay[i]->SetTexture(mCurrent.frames[i].image);
            mEngine->Draw(mOverlay[i]);
        }
        DrawStatistics();
        mEngine->DisplayColorBuffer(0);
    }
#endif

#if defined(DO_TRIGGERED_SERIAL) || defined(DO_TRIGGERED_PARALLEL)
    if (mVideoStreamManager->GetFrame(mCurrent))
    {
        for (int i = 0; i < 4; ++i)
        {
            mOverlay[i]->SetTexture(mCurrent.frames[i].image);
            mEngine->Draw(mOverlay[i]);
        }
        DrawStatistics();
        mEngine->DisplayColorBuffer(0);
    }
#endif
}

bool VideoStreamsWindow2::OnCharPress(unsigned char key, int x, int y)
{
    if (key == ' ')
    {
        mTimer.Reset();
        mVideoStreamManager->ResetPerformanceMeasurements();
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}

bool VideoStreamsWindow2::CreateOverlays(int textureWidth, int textureHeight)
{
    // Use nearest filtering and clamped texture coordinates.
    SamplerState::Filter filter = SamplerState::MIN_P_MAG_P_MIP_P;
    SamplerState::Mode mode = SamplerState::CLAMP;
    mOverlay[0] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, textureWidth, textureHeight, filter, mode, mode, true);
    std::array<int, 4> rect0 = { 0, 0, mXSize / 2, mYSize / 2 };
    mOverlay[0]->SetOverlayRectangle(rect0);

    mOverlay[1] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, textureWidth, textureHeight, filter, mode, mode, true);
    std::array<int, 4> rect1 = { mXSize / 2, 0, mXSize / 2, mYSize / 2 };
    mOverlay[1]->SetOverlayRectangle(rect1);

    mOverlay[2] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, textureWidth, textureHeight, filter, mode, mode, true);
    std::array<int, 4> rect2 = { 0, mYSize / 2, mXSize / 2, mYSize / 2 };
    mOverlay[2]->SetOverlayRectangle(rect2);

    mOverlay[3] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, textureWidth, textureHeight, filter, mode, mode, true);
    std::array<int, 4> rect3 =
        { mXSize / 2, mYSize / 2, mXSize / 2, mYSize / 2 };
    mOverlay[3]->SetOverlayRectangle(rect3);

    // Create a black texture for the initial drawing of the window.
    auto texture = std::make_shared<Texture2>(DF_R16_UNORM, textureWidth, textureHeight);
    texture->SetName("black texture");
    std::memset(texture->GetData(), 0, texture->GetNumBytes());
    mEngine->Bind(texture);
    mOverlay[0]->SetTexture(texture);
    mOverlay[1]->SetTexture(texture);
    mOverlay[2]->SetTexture(texture);
    mOverlay[3]->SetTexture(texture);
    return true;
}

void VideoStreamsWindow2::DrawStatistics()
{
    double averageTime, averageVSMTime;
    std::vector<double> averageVSTime(NUM_VIDEO_STREAMS);
    mVideoStreamManager->GetStatistics(averageTime, averageVSMTime, averageVSTime);

    std::array<float, 4> textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    std::string message = "frame: " + std::to_string(mCurrent.number);
    mEngine->Draw(8, mYSize - 56, textColor, message);

    message = "vsm average frame msec: " + std::to_string(averageTime);
    mEngine->Draw(8, mYSize - 40, textColor, message);

    message = "vsm average capture msec: " + std::to_string(averageVSMTime);
    mEngine->Draw(8, mYSize - 24, textColor, message);

    message = "vs average capture msec: ";
    for (int i = 0; i < NUM_VIDEO_STREAMS; ++i)
    {
        message += ", vs" + std::to_string(i) +" = " + std::to_string(averageVSTime[i]);
    }
    mEngine->Draw(8, mYSize - 8, textColor, message);
}
