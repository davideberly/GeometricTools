// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include "VisualizeMarchingCubesWindow3.h"
#include <iostream>

int main()
{
    try
    {
        Window::Parameters parameters(L"VisualizeMarchingCubesWindow3", 0, 0, 512, 512);
        auto window = TheWindowSystem.Create<VisualizeMarchingCubesWindow3>(parameters);
        TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
        TheWindowSystem.Destroy(window);
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}

