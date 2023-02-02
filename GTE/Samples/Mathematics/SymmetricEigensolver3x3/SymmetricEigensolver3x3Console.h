// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Console.h>
using namespace gte;

class SymmetricEigensolver3x3Console : public Console
{
public:
    SymmetricEigensolver3x3Console(Parameters& parameters);

    virtual void Execute() override;

private:
    static double Determinant(std::array<std::array<double, 3>, 3> const& evec);
};
