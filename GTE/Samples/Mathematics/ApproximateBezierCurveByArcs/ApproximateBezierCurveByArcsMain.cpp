// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.05.05

#include "ApproximateBezierCurveByArcsWindow2.h"
#include <iostream>

int main()
{
    try
    {
        Window::Parameters parameters(L"ApproximateBezierCurveByArcsWindow2", 0, 0, 512, 512);
        auto window = TheWindowSystem.Create<ApproximateBezierCurveByArcsWindow2>(parameters);
        TheWindowSystem.MessagePump(window, TheWindowSystem.NO_IDLE_LOOP);
        TheWindowSystem.Destroy(window);
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
