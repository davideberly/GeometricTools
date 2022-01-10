// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#if 1
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Timer.h>

//#define USE_DEL3_OLD
//#define USE_DEL3_NEW_TSMANI
#define USE_DEL3_NEW_TSMANI_INTERVAL

#if defined(USE_DEL3_OLD)
#include <Mathematics/Delaunay3.h>
#endif

#if defined(USE_DEL3_NEW_TSMANI)
#include "Delaunay3A.h"
#endif

#if defined(USE_DEL3_NEW_TSMANI_INTERVAL)
#include "Delaunay3B.h"
#endif

using Numeric = float;

#include <fstream>
#include <iostream>
using namespace gte;

int main()
{
    try
    {
        std::vector<Vector3<Numeric>> vertices(3066);
        std::ifstream input("sandeep.txt");
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            input >> vertices[i][0] >> vertices[i][1] >> vertices[i][2];
        }
        input.close();

        Vector3<Numeric> pmin{}, pmax{};
        ComputeExtremes((int)vertices.size(), vertices.data(), pmin, pmax);
        Vector3<Numeric> center = (Numeric)0.5 * (pmin + pmax);
        Vector3<Numeric> extreme = (Numeric)0.5 * (pmax - pmin);
        for (auto& p : vertices)
        {
            p -= center;
            p[0] /= extreme[0];
            p[1] /= extreme[1];
            p[2] /= extreme[2];
        }

        Timer timer{};
#if defined(USE_DEL3_OLD)
        Delaunay3<Numeric, BSNumber<UIntegerFP32<44>>> delaunay{};
        delaunay((int)vertices.size(), vertices.data(), (Numeric)0);
#endif

#if defined(USE_DEL3_NEW_TSMANI)
        Delaunay3A<Numeric, BSNumber<UIntegerFP32<44>>> delaunay{};
        delaunay((int)vertices.size(), vertices.data(), (Numeric)0);
#endif

#if defined(USE_DEL3_NEW_TSMANI_INTERVAL)
        Delaunay3B<Numeric> delaunay{};
        delaunay(vertices);
#endif

        auto msecs = timer.GetMilliseconds();
        std::cout << "time = " << msecs << " milliseconds" << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}

#else
#include "Delaunay3DWindow3.h"
#include <iostream>

int main()
{
    try
    {
        Window::Parameters parameters(L"Delaunay3DWindow3", 0, 0, 512, 512);
        auto window = TheWindowSystem.Create<Delaunay3DWindow3>(parameters);
        TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
        TheWindowSystem.Destroy(window);
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
#endif
