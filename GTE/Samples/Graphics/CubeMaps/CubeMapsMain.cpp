// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#include "CubeMapsWindow3.h"
#include <Applications/LogReporter.h>

int main()
{
#if defined(_DEBUG)
    // TODO: The message-box logger is not enabled.  The initial draw calls
    // for the 6 walls generate OpenGL errors in the glGetUniformLocation of
    // GL4Engine::EnableTextures.  The errors are GL_INVALID_VALUE, but yet
    // the program handles are valid.  On subsequent draw calls, the OpenGL
    // errors are NOT generated.  This behavior occurs on AMD or NVIDIA
    // hardware.  We need to diagnose the problem.
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_NOTHING,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

    Window::Parameters parameters(L"CubeMapsWindow3", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<CubeMapsWindow3>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy(window);
    return 0;
}
