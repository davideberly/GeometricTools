// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.07.04

#include "Fluids2DWindow2.h"
#include <iostream>

int32_t main()
{
    try
    {

#if defined(SAVE_RENDERING_TO_DISK)
        Window::Parameters parameters(L"Fluids2DWindow2", 0, 0,
            Fluids2DWindow2::GRID_SIZE, Fluids2DWindow2::GRID_SIZE);
#else
        Window::Parameters parameters(L"Fluids2DWindow2", 0, 0, 768, 768);
        parameters.allowResize = true;
#endif

        auto window = TheWindowSystem.Create<Fluids2DWindow2>(parameters);
        TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
        TheWindowSystem.Destroy(window);
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
