// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
#include "VideoStreamManager.h"
#include "FileVideoStream.h"
using namespace gte;

// NOTE: Expose only one of these.
#if defined(GTE_USE_OPENGL)
// TODO:  The only flag that works currently with GL45 is DO_MANUAL_SERIAL,
// because the OpenGL engine is not thread-safe regarding resource
// creation (the DX11 engine is).  Add thread-safe resource creation
// to GL45 by supporting sharing via contexts.
#define DO_MANUAL_SERIAL
#endif
#if defined(GTE_USE_DIRECTX)
#define DO_MANUAL_SERIAL
//#define DO_MANUAL_PARALLEL
//#define DO_TRIGGERED_SERIAL
//#define DO_TRIGGERED_PARALLEL
#endif

class VideoStreamsWindow2 : public Window2
{
public:
    virtual ~VideoStreamsWindow2();
    VideoStreamsWindow2(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool CreateOverlays(int32_t textureWidth, int32_t textureHeight);
    void DrawStatistics();

    enum { NUM_VIDEO_STREAMS = 4 };
    std::vector<std::shared_ptr<VideoStream>> mVideoStreams;
    std::vector<std::shared_ptr<OverlayEffect>> mOverlay;
    std::unique_ptr<VideoStreamManager> mVideoStreamManager;
    VideoStreamManager::Frame mCurrent;
};
