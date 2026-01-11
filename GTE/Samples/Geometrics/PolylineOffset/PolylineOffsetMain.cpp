// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include "PolylineOffsetWindow2.h"
#include <iostream>

int main()
{
    try
    {
        Window::Parameters parameters(L"PolylineOffsetWindow2", 0, 0, 768, 768);
        auto window = TheWindowSystem.Create<PolylineOffsetWindow2>(parameters);
        TheWindowSystem.MessagePump(window, TheWindowSystem.NO_IDLE_LOOP);
        TheWindowSystem.Destroy(window);
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}


