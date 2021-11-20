// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 4.5.2021.11.12

#include "FitSqrt.h"
#include "FitInvSqrt.h"
#include "FitSin.h"
#include "FitCos.h"
#include "FitTan.h"
#include "FitASin.h"
#include "FitATan.h"
#include "FitExp2.h"
#include "FitLog2.h"
#include "FitReciprocal.h"
#include <iostream>

int main()
{
    try
    {
        FitSqrt fitterSqrt;
        FitInvSqrt fitterInvSqrt;
        FitSin fitterSin;  // template parameter is 'order', degree = 2*order + 1
        FitCos fitterCos;  // template parameter is 'order', degree = 2*order
        FitTan fitterTan;  // template parameter is 'order', degree = 2*order + 1
        FitASin fitterASin;  // template parameter is 'order', degree = 2*order + 1
        FitATan fitterATan;  // template parameter is 'order', degree = 2*order + 1
        FitExp2 fitterExp2;
        FitLog2 fitterLog2;
        FitReciprocal fitterReciprocal;

        // Generate an approximation for sin(x) of degree 9 (= 2 * 4 + 1).
        std::vector<double> poly;
        double error;
        fitterSin.Generate<4>(poly, error);
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}

