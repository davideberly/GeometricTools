// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/ThreadSafeQueue.h>
#include "VideoStream.h"
#include <thread>

class VideoStreamManager
{
public:
    // Construction and destruction.  The input videoStreams must have at
    // least one element and all elements must be nonnull.  NOTE:  No
    // error checking is performed for this condition.  The manager maintains
    // a queue of pending frames to process.  You must specify the maximum
    // number.  In the producer-consumer model where the producer and
    // consumer keep up with each other, the number of elements in the queue
    // is 0 or 1.
    ~VideoStreamManager();
    VideoStreamManager(std::vector<std::shared_ptr<VideoStream>> const& videoStreams,
        size_t maxQueueElements);

    // Access to the managed video streams.
    inline std::vector<std::shared_ptr<VideoStream>> const& GetVideoStreams() const
    {
        return mVideoStreams;
    }

    // A frame consists of a frame number (unique identifier), a collection of
    // frames from the video streams, and the time (in microseconds) to capture
    // all frames from the video streams.
    struct Frame
    {
        Frame(size_t n)
            :
            number(0xFFFFFFFF),
            frames(n),
            microseconds(0)
        {
        }

        uint32_t number;
        std::vector<VideoStream::Frame> frames;
        int64_t microseconds;
    };

    // Get the current frame.  The return value is 'true' iff the frame
    // queue is not empty, in which case 'frame' is valid.
    inline bool GetFrame(Frame& frame) const
    {
        return mFrameQueue.Pop(frame);
    }

    // Support for production of a single frame of a collection of video
    // streams.  The functions return 'true' iff the prodcution was successful
    // (all video streams returned a frame).

    // Capture an image from each video stream so that they run in series.
    void CaptureFrameSerial();

    // Launch a thread per video stream so that they effectively run in
    // parallel and then wait until all are finished (for synchronization).
    void CaptureFrameParallel();

    // The following functions allow the capture functions to be called at the
    // specified frames per second (fps).  Start*(fps) launches a thread that
    // uses a timer to decide when to capture.  Stop*() destroys the thread.
    // Set 'parallel' to 'true' to have CaptureFrameParallel() called;
    // otherwise, set it to 'false' to have CaptureFrameSerial() called.
    void StartTriggeredCapture(double fps, bool parallel);
    void StopTriggeredCapture();

    // Performance measurements.  These are accumulated measurements starting
    // from a call to ResetPerformanceMeasurements().
    void ResetPerformanceMeasurements();

    inline uint32_t GetPerformanceFrames() const
    {
        return mPerformanceFrames;
    }

    inline int64_t GetPerformanceMicroseconds() const
    {
        return mPerformanceMicroseconds;
    }

    double GetFramesPerSecond() const;
    double GetSecondsPerFrame() const;

    // Compute the:
    // 1. average number of milliseconds per frame, end-to-end
    //      (capture time + sleep time).
    // 2. average number of milliseconds per frame of capture time only.
    // 3. average number of milliseconds per frame of capture time only
    //      for each video stream.
    void GetStatistics(double& averageTime, double& averageVSMTime,
        std::vector<double>& averageVSTime);

protected:
    // Common code for captures.
    void AssembleFullFrame(int64_t startMicroseconds);

    // The managed video streams.
    std::vector<std::shared_ptr<VideoStream>> mVideoStreams;

    // The queue of frames to process.
    mutable gte::ThreadSafeQueue<Frame> mFrameQueue;

    // The timer is used to compute how long it takes to produce the frame.
    // The current frame counter is used for the Frame.number member.
    gte::Timer mProductionTimer;
    uint32_t mCurrentFrame;

    // Support for triggered capture.
    struct Trigger
    {
        Trigger()
            :
            timer{},
            microsecondsPerFrame(0),
            running(false),
            triggerThread{}
        {
        }

        gte::Timer timer;
        int64_t microsecondsPerFrame;
        bool running;
        std::unique_ptr<std::thread> triggerThread;
    };

    std::unique_ptr<Trigger> mTrigger;

    // Performance measurements.
    gte::Timer mPerformanceTimer;
    uint32_t mPerformanceFrames;
    int64_t mPerformanceMicroseconds;
    int64_t mAccumulatedVSMMicroseconds;
    std::vector<int64_t> mAccumulatedVSMicroseconds;
};
