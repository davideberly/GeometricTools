// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.11

#include "WrigglingSnakeWindow3.h"
#include <iostream>

int main()
{
    try
    {
        Window::Parameters parameters(L"WrigglingSnakeWindow3", 0, 0, 640, 480);
        auto window = TheWindowSystem.Create<WrigglingSnakeWindow3>(parameters);
        TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
        TheWindowSystem.Destroy(window);
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
