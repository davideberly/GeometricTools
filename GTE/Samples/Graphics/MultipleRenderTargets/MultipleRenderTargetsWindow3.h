// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
using namespace gte;

class MultipleRenderTargetsWindow3 : public Window3
{
public:
    MultipleRenderTargetsWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    bool CreateScene();
    void CreateOverlays();

    std::string mSourceFileVS;
    std::string mSourceFilePS;

    std::shared_ptr<DrawTarget> mDrawTarget;
    std::shared_ptr<Visual> mSquare;
    std::shared_ptr<Texture2> mLinearDepth;
    std::shared_ptr<OverlayEffect> mOverlay[7];
    uint32_t mActiveOverlay;

    // Shader source code as strings.
    static std::string const msGLSLOverlayPSSource[5];
    static std::string const msHLSLOverlayPSSource[5];
    static ProgramSources const msOverlayPSSource[5];
};
