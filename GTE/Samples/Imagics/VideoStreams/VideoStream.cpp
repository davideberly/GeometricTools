// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "VideoStream.h"
using namespace gte;

VideoStream::VideoStream(std::shared_ptr<gte::GraphicsEngine> const& engine)
    :
    mEngine(engine),
    mType(DF_UNKNOWN),
    mWidth(0),
    mHeight(0),
    mPerformanceFrames(0),
    mPerformanceMicroseconds(0)
{
}

void VideoStream::CaptureFrame()
{
    int64_t startMicroseconds = mProductionTimer.GetMicroseconds();

    char* data = GetImage();
    if (data)
    {
        // The texture is created without system memory.  The derived class
        // provides this data, so the texture is given temporary access to
        // it in order for the Bind(...) call to copy the data to the GPU.
        mFrame.image = std::make_shared<Texture2>(mType, mWidth, mHeight, false, false);
        mFrame.image->SetData(data);
        mEngine->Bind(mFrame.image);
        mFrame.image->SetData(nullptr);
    }
    // else: GetImage has signaled that there is no image available.

    int64_t finalMicroseconds = mProductionTimer.GetMicroseconds();
    mFrame.microseconds = finalMicroseconds - startMicroseconds;

    mPerformanceMicroseconds = mPerformanceTimer.GetMicroseconds();
    ++mPerformanceFrames;
}

void VideoStream::ResetPerformanceMeasurements()
{
    mPerformanceFrames = 0;
    mPerformanceMicroseconds = 0;
    mPerformanceTimer.Reset();
}

double VideoStream::GetFramesPerSecond() const
{
    if (mPerformanceMicroseconds > 0)
    {
        double seconds =
            static_cast<double>(mPerformanceMicroseconds) / 1000000.0;
        return static_cast<double>(mPerformanceFrames) / seconds;
    }
    return 0.0;
}

double VideoStream::GetSecondsPerFrame() const
{
    if (mPerformanceFrames > 0)
    {
        double seconds =
            static_cast<double>(mPerformanceMicroseconds) / 1000000.0;
        return seconds / static_cast<double>(mPerformanceFrames);
    }
    return 0.0;
}
