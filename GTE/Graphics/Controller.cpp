// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Controller.h>
#include <Mathematics/Math.h>
using namespace gte;

Controller::Controller()
    :
    repeat(RepeatType::CLAMP),
    minTime(0.0),
    maxTime(0.0),
    phase(0.0),
    frequency(1.0),
    active(true),
    name(""),
    mObject(nullptr),
    mApplicationTime(-std::numeric_limits<double>::max())
{
}

Controller::~Controller()
{
}

bool Controller::Update(double applicationTime)
{
    if (active)
    {
        mApplicationTime = applicationTime;
        return true;
    }
    return false;
}

void Controller::SetObject(ControlledObject* object)
{
    mObject = object;
}

double Controller::GetControlTime(double applicationTime)
{
    double controlTime = frequency * applicationTime + phase;

    if (repeat == RepeatType::CLAMP)
    {
        // Clamp the time to the [min,max] interval.
        if (controlTime < minTime)
        {
            return minTime;
        }
        if (controlTime > maxTime)
        {
            return maxTime;
        }
        return controlTime;
    }

    double timeRange = maxTime - minTime;
    if (timeRange > 0.0)
    {
        double multiples = (controlTime - minTime) / timeRange;
        double integerTime = std::floor(multiples);
        double fractionTime = multiples - integerTime;
        if (repeat == RepeatType::WRAP)
        {
            return minTime + fractionTime*timeRange;
        }

        // repeat == RepeatType::CYCLE
        if (static_cast<int32_t>(integerTime) & 1)
        {
            // Go backward in time.
            return maxTime - fractionTime * timeRange;
        }
        else
        {
            // Go forward in time.
            return minTime + fractionTime * timeRange;
        }
    }

    // The minimum and maximum times are the same, so return the minimum.
    return minTime;
}
