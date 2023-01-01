// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BeadSlideConsole.h"
#include "PhysicsModule.h"
#include <fstream>
#include <sstream>
#include <iomanip>

BeadSlideConsole::BeadSlideConsole(Parameters& parameters)
    :
    Console(parameters)
{
}

void BeadSlideConsole::Execute()
{
    // Set up the physics module.
    PhysicsModule module;
    module.gravity = 1.0f;
    module.mass = 0.1f;

    float time = 0.0f;
    float deltaTime = 0.001f;
    float q = 1.0f;
    float qDot = 0.0f;
    module.Initialize(time, deltaTime, q, qDot);

    // Run the simulation.
    std::ofstream outFile("simulation.txt");
    outFile << "time   q            qder         position" << std::endl;
    int32_t const imax = 2500;
    for (int32_t i = 0; i < imax; ++i)
    {
        float x = q, y = q * q, z = q * y;

        std::ostringstream message;
        message << std::fixed << std::showpoint;
        message << std::setw(5) << std::setprecision(3) << time << ' ';
        message << std::showpos;
        message << std::setw(12) << std::setprecision(8) << q << ' ';
        message << std::setw(12) << std::setprecision(8) << qDot << ' ';
        message << std::setw(8) << std::setprecision(4) << x << ' ';
        message << std::setw(8) << std::setprecision(4) << y << ' ';
        message << std::setw(8) << std::setprecision(4) << z;
        outFile << message.str() << std::endl;

        module.Update();
        time = module.GetTime();
        q = module.GetQ();
        qDot = module.GetQDot();
    }
    outFile.close();
}
