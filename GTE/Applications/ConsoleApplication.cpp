// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Applications/GTApplicationsPCH.h>
#include <Applications/ConsoleApplication.h>
using namespace gte;

ConsoleApplication::Parameters::Parameters()
    :
    title(L""),
    created(false)
{
}

ConsoleApplication::Parameters::Parameters(std::wstring const& inTitle)
    :
    title(inTitle),
    created(false)
{
}

ConsoleApplication::ConsoleApplication(Parameters const& parameters)
    :
    Application(parameters),
    mTitle(parameters.title)
{
}

