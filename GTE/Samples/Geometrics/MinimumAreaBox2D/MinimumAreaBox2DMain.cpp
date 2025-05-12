// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include "MinimumAreaBox2DWindow2.h"
#include <iostream>

int32_t main()
{
    try
    {
        Window::Parameters parameters(L"MinimumAreaBox2DWindow2", 0, 0, 512, 512);
        auto window = TheWindowSystem.Create<MinimumAreaBox2DWindow2>(parameters);
        TheWindowSystem.MessagePump(window, TheWindowSystem.NO_IDLE_LOOP);
        TheWindowSystem.Destroy(window);
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}

