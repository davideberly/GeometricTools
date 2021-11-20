// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.8.2021.11.12

#include "RotationApproximationConsole.h"
#include <iostream>

int main()
{
    try
    {
        Console::Parameters parameters(L"RotationApproximationConsole");
        auto console = TheConsoleSystem.Create<RotationApproximationConsole>(parameters);
        TheConsoleSystem.Execute(console);
        TheConsoleSystem.Destroy(console);
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
