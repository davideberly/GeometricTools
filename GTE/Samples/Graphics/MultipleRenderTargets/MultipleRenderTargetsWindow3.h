// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
using namespace gte;

class MultipleRenderTargetsWindow3 : public Window3
{
public:
    MultipleRenderTargetsWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

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
    unsigned int mActiveOverlay;

    // Shader source code as strings.
    static std::string const msGLSLOverlayPSSource[5];
    static std::string const msHLSLOverlayPSSource[5];
    static ProgramSources const msOverlayPSSource[5];
};
