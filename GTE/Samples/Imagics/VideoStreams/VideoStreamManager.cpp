// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "VideoStreamManager.h"
using namespace gte;

VideoStreamManager::~VideoStreamManager()
{
    StopTriggeredCapture();
}

VideoStreamManager::VideoStreamManager(std::vector<std::shared_ptr<VideoStream>> const& videoStreams,
    size_t maxQueueElements)
    :
    mVideoStreams(videoStreams),
    mFrameQueue(maxQueueElements),
    mCurrentFrame(0),
    mTrigger(nullptr),
    mPerformanceFrames(0),
    mPerformanceMicroseconds(0),
    mAccumulatedVSMMicroseconds(0),
    mAccumulatedVSMicroseconds(videoStreams.size())
{
    ResetPerformanceMeasurements();
}

void VideoStreamManager::CaptureFrameSerial()
{
    int64_t startMicroseconds = mProductionTimer.GetMicroseconds();

    // Capture a frame for each video stream.
    size_t const numVideoStreams = mVideoStreams.size();
    for (size_t i = 0; i < numVideoStreams; ++i)
    {
        mVideoStreams[i]->CaptureFrame();
    }

    AssembleFullFrame(startMicroseconds);
}

void VideoStreamManager::CaptureFrameParallel()
{
    int64_t startMicroseconds = mProductionTimer.GetMicroseconds();

    // Launch capture threads for all video streams.
    size_t const numVideoStreams = mVideoStreams.size();
    std::vector<std::thread> captureThread(numVideoStreams);
    for (size_t i = 0; i < numVideoStreams; ++i)
    {
        captureThread[i] = std::thread
        (
            [this, i]()
            {
                mVideoStreams[i]->CaptureFrame();
            }
        );
    }

    // Wait for all video streams to capture their images.
    for (size_t i = 0; i < numVideoStreams; ++i)
    {
        captureThread[i].join();
    }

    AssembleFullFrame(startMicroseconds);
}

void VideoStreamManager::StartTriggeredCapture(double fps, bool parallel)
{
    if (nullptr == mTrigger && fps > 0.0)
    {
        void (VideoStreamManager::*Capture)(void);
        if (parallel)
        {
            Capture = &VideoStreamManager::CaptureFrameParallel;
        }
        else
        {
            Capture = &VideoStreamManager::CaptureFrameSerial;
        }

        mTrigger = std::make_unique<Trigger>();
        mTrigger->microsecondsPerFrame = static_cast<int64_t>(1000000.0 / fps);
        mTrigger->running = true;

        mTrigger->triggerThread = std::make_unique<std::thread>
        (
            [this, Capture]()
            {
                int64_t startTime = mTrigger->timer.GetMicroseconds();

                // The variable 'running' is set to 'false' in the function
                // StopTriggeredCapture().  Access here to 'running' is not
                // protected by a lock in order to help with performance.  The
                // worst case is that the trigger fires several times after
                // 'running' is set to 'false' (on program termination) but
                // before the read here notices the change.
                while (mTrigger->running)
                {
                    // Spin in the thread for an accurate 'sleep' time.
                    int64_t finalTime =
                        startTime + mTrigger->microsecondsPerFrame;
                    do
                    {
                        startTime = mTrigger->timer.GetMicroseconds();
                    }
                    while (startTime < finalTime);

                    (this->*Capture)();
                }
            }
        );
    }
}

void VideoStreamManager::StopTriggeredCapture()
{
    if (mTrigger && mTrigger->triggerThread)
    {
        mTrigger->running = false;
        mTrigger->triggerThread->join();
        mTrigger->triggerThread = nullptr;
        mTrigger = nullptr;
    }
}

void VideoStreamManager::ResetPerformanceMeasurements()
{
    for (auto const& vs : mVideoStreams)
    {
        vs->ResetPerformanceMeasurements();
    }

    mPerformanceFrames = 0;
    mPerformanceMicroseconds = 0;
    mPerformanceTimer.Reset();

    mAccumulatedVSMMicroseconds = 0;
    for (auto& ticks : mAccumulatedVSMicroseconds)
    {
        ticks = 0;
    }
}

double VideoStreamManager::GetFramesPerSecond() const
{
    if (mPerformanceMicroseconds > 0)
    {
        double seconds =
            static_cast<double>(mPerformanceMicroseconds) / 1000000.0;
        return static_cast<double>(mPerformanceFrames) / seconds;
    }
    return 0.0;
}

double VideoStreamManager::GetSecondsPerFrame() const
{
    if (mPerformanceFrames > 0)
    {
        double seconds =
            static_cast<double>(mPerformanceMicroseconds) / 1000000.0;
        return seconds / static_cast<double>(mPerformanceFrames);
    }
    return 0.0;
}

void VideoStreamManager::GetStatistics(double& averageTime,
    double& averageVSMTime, std::vector<double>& averageVSTime)
{
    averageVSTime.resize(mVideoStreams.size());

    if (mPerformanceMicroseconds > 0)
    {
        double invNumFrames = 1.0 / static_cast<double>(mPerformanceFrames);

        double msecs = static_cast<double>(mPerformanceMicroseconds) / 1000.0;
        averageTime = msecs * invNumFrames;

        msecs = static_cast<double>(mAccumulatedVSMMicroseconds) / 1000.0;
        averageVSMTime = msecs * invNumFrames;

        for (size_t i = 0; i < mVideoStreams.size(); ++i)
        {
            msecs = static_cast<double>(mAccumulatedVSMicroseconds[i]) / 1000.0;
            averageVSTime[i] = msecs * invNumFrames;
        }
    }
    else
    {
        averageTime = 0.0;
        averageVSMTime = 0.0;
        for (size_t i = 0; i < mVideoStreams.size(); ++i)
        {
            averageVSTime[i] = 0.0;
        }
    }
}

void VideoStreamManager::AssembleFullFrame(int64_t startMicroseconds)
{
    size_t const numVideoStreams = mVideoStreams.size();
    Frame full(numVideoStreams);
    for (size_t i = 0; i < numVideoStreams; ++i)
    {
        full.frames[i] = mVideoStreams[i]->GetFrame();
    }

    int64_t finalMicroseconds = mProductionTimer.GetMicroseconds();
    full.number = mCurrentFrame++;
    full.microseconds = finalMicroseconds - startMicroseconds;
    mFrameQueue.Push(full);

    mPerformanceMicroseconds = mPerformanceTimer.GetMicroseconds();
    ++mPerformanceFrames;

    mAccumulatedVSMMicroseconds += full.microseconds;
    for (size_t i = 0; i < numVideoStreams; ++i)
    {
        mAccumulatedVSMicroseconds[i] += full.frames[i].microseconds;
    }
}
