// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#include "Fluids2DWindow2.h"
#include <Applications/LogReporter.h>

int main()
{
#if defined(_DEBUG)
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

#if defined(SAVE_RENDERING_TO_DISK)
    Window::Parameters parameters(L"Fluids2DWindow2", 0, 0,
        Fluids2DWindow2::GRID_SIZE, Fluids2DWindow2::GRID_SIZE);
#else
    Window::Parameters parameters(L"Fluids2DWindow2", 0, 0, 768, 768);
#endif

    auto window = TheWindowSystem.Create<Fluids2DWindow2>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy(window);
    return 0;
}
