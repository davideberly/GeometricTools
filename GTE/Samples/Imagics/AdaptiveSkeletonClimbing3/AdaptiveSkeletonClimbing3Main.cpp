// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include "AdaptiveSkeletonClimbing3Console.h"
#include <iostream>

int32_t main()
{
    try
    {
        Console::Parameters parameters(L"AdaptiveSkeletonClimbing3Console");
        auto console = TheConsoleSystem.Create<AdaptiveSkeletonClimbing3Console>(parameters);
        TheConsoleSystem.Execute(console);
        TheConsoleSystem.Destroy(console);
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}


