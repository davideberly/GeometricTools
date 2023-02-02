// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "GpuGaussianBlur2Window2.h"
#include <Applications/Command.h>
#include <iostream>

int32_t main(int32_t numArguments, char* arguments[])
{
    try
    {
        Command command(numArguments, arguments);
        bool useDirichlet = (command.GetBoolean("d") > 0 ? true : false);

        // The window size is that of the Head_U16_X256_Y256.binary image.
        GpuGaussianBlur2Window2::Parameters parameters(L"GpuGaussianBlur2Window2",
            0, 0, 256, 256, useDirichlet);
        auto window = TheWindowSystem.Create<GpuGaussianBlur2Window2>(parameters);
        TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
        TheWindowSystem.Destroy(window);
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
