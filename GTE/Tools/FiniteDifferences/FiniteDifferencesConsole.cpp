#include "FiniteDifferencesConsole.h"

FiniteDifferencesConsole::FiniteDifferencesConsole(Parameters& parameters)
    :
    Console(parameters)
{
}

void FiniteDifferencesConsole::Execute()
{
    //GenerateM1P1();
    //GenerateM1P2();
    //GenerateM1P3();
    //GenerateM1P4();

    //GenerateM2P1();
    //GenerateM2P2();
    //GenerateM2P3();
    //GenerateM2P4();

    //GenerateM3P1();
    //GenerateM3P2();
    //GenerateM3P3();
    //GenerateM3P4();

    //GenerateM4P1();
    //GenerateM4P2();
    //GenerateM4P3();
    GenerateM4P4();
}

void FiniteDifferencesConsole::GenerateM1P1()
{
    bool success;
    std::array<Rational, 2> C;
    std::array<double, 2> dC;
    int errorOrder;

    // (imin,imax) = (0,1)
    success = Generate<1, 1>(0, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 2; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_0,C_1) = (-1,1)
    // errorOrder = 1

    // (imin,imax) = (1,2)
    success = Generate<1, 1>(1, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 2; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_1,C_2) = (-1,1)
    // errorOrder = 1

    // (imin,imax) = (-1,0)
    success = Generate<1, 1>(-1, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 2; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-1},C_0) = (-1,1)
    // errorOrder = 1

    // (imin,imax) = (-2,-1)
    success = Generate<1, 1>(-2, -1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 2; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-2},C_{-1}) = (-1,1)
    // errorOrder = 1

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM1P2()
{
    bool success;
    std::array<Rational, 3> C;
    std::array<double, 3> dC;
    int errorOrder;

    Rational two(2);

    // (imin,imax) = (0,2)
    success = Generate<1, 2>(0, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 3; ++i)
        {
            dC[i] = two * C[i];
        }
    }
    // (C0,C1,C2) = (-3,4,-1)/2
    // errorOrder = 2

    // (imin,imax) = (1,3)
    success = Generate<1, 2>(1, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 3; ++i)
        {
            dC[i] = two * C[i];
        }
    }
    // (C_1,C_2,C_3) = (-5,8,-3)/2
    // errorOrder = 2

    // (imin,imax) = (-2,0)
    success = Generate<1, 2>(-2, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 3; ++i)
        {
            dC[i] = two * C[i];
        }
    }
    // (C_{-2},C_{-1},C_0) = (1,-4,3)/2
    // errorOrder = 2

    // (imin,imax) = (-3,-1)
    success = Generate<1, 2>(-3, -1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 3; ++i)
        {
            dC[i] = two * C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1}) = (3,-8,5)/2
    // errorOrder = 2

    // (imin,imax) = (-1,1)
    success = Generate<1, 2>(-1, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 3; ++i)
        {
            dC[i] = two * C[i];
        }
    }
    // (C_{-1},C_0,C_1) = (-1,0,1)/2
    // errorOrder = 2

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM1P3()
{
    bool success;
    std::array<Rational, 4> C;
    std::array<double, 4> dC;
    int errorOrder;

    Rational six(6);

    // (imin,imax) = (0,3)
    success = Generate<1, 3>(0, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_0,C_1,C_2,C_3) = (-11,18,-9,2)/6
    // errorOrder = 3

    // (imin,imax) = (-1,2)
    success = Generate<1, 3>(-1, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2) = (-2,-3,6,-1)/6
    // errorOrder = 3

    // (imin,imax) = (-2,1)
    success = Generate<1, 3>(-2, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1) = (1,-6,3,2)/6
    // errorOrder = 3

    // (imin,imax) = (-3,0)
    success = Generate<1, 3>(-3, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_0) = (-2,9,-18,11)/6
    // errorOrder = 3

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM1P4()
{
    bool success;
    std::array<Rational, 5> C;
    std::array<double, 5> dC;
    int errorOrder;

    Rational twelve(12);

    // (imin,imax) = (0,4)
    success = Generate<1, 4>(0, 4, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_0,C_1,C_2,C_3,C_4) = (-25,48,-36,16,-3)/12
    // errorOrder = 4

    // (imin,imax) = (-1,3)
    success = Generate<1, 4>(-1, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2,C_3) = (-3,-10,18,-6,1)/12
    // errorOrder = 4

    // (imin,imax) = (-2,2)
    success = Generate<1, 4>(-2, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1,C_2) = (1,-8,0,8,-1)/12
    // errorOrder = 4

    // (imin,imax) = (-3,1)
    success = Generate<1, 4>(-3, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_0,C_1) = (-1,6,-18,10,3)/12
    // errorOrder = 4

    // (imin,imax) = (-4,0)
    success = Generate<1, 4>(-4, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-4},C_{-3},C_{-2},C_{-1},C_0) = (3,-16,36,-48,25)/12
    // errorOrder = 4

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM2P1()
{
    bool success;
    std::array<Rational, 3> C;
    std::array<double, 3> dC;
    int errorOrder;

    // (imin,imax) = (0,2)
    success = Generate<2, 1>(0, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 3; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_0,C_1,C_2) = (1,-2,1)
    // errorOrder = 1

    // (imin,imax) = (-1,1)
    success = Generate<2, 1>(-1, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 3; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-1},C_0,C_1) = (1,-2,1)
    // errorOrder = 2

    // (imin,imax) = (-2,0)
    success = Generate<2, 1>(-2, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 3; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-2},C_{-1},C_0) = (1,-2,1)
    // errorOrder = 1

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM2P2()
{
    bool success;
    std::array<Rational, 4> C;
    std::array<double, 4> dC;
    int errorOrder;

    // (imin,imax) = (0,3)
    success = Generate<2, 2>(0, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_0,C_1,C_2,C_3) = (2,-5,4,-1)
    // errorOrder = 2

    // (imin,imax) = (-1,2)
    success = Generate<2, 2>(-1, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2) = (1,-2,1,0)
    // errorOrder = 2

    // (imin,imax) = (-2,1)
    success = Generate<2, 2>(-2, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1) = (0,1,-2,1)
    // errorOrder = 2

    // (imin,imax) = (-3,0)
    success = Generate<2, 2>(-3, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_0) = (-1,4,-5,2)
    // errorOrder = 2

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM2P3()
{
    bool success;
    std::array<Rational, 5> C;
    std::array<double, 5> dC;
    int errorOrder;

    Rational twelve(12);

    // (imin,imax) = (0,4)
    success = Generate<2, 3>(0, 4, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_0,C_1,C_2,C_3,C_4) = (35,-104,114,-56,-11)/12
    // errorOrder = 3

    // (imin,imax) = (-1,3)
    success = Generate<2, 3>(-1, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2,C_3) = (11,-20,6,4,-1)/12
    // errorOrder = 3

    // (imin,imax) = (-2,2)
    success = Generate<2, 3>(-2, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1,C_2) = (-1,16,-30,16,-1)/12
    // errorOrder = 4

    // (imin,imax) = (-3,1)
    success = Generate<2, 3>(-3, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_0,C_1) = (-1,4,6,-20,11)/12
    // errorOrder = 3

    // (imin,imax) = (-4,0)
    success = Generate<2, 3>(-4, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-4},C_{-3},C_{-2},C_{-1},C_0) = (11,-56,114,-104,35)/12
    // errorOrder = 3

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM2P4()
{
    bool success;
    std::array<Rational, 6> C;
    std::array<double, 6> dC;
    int errorOrder;

    Rational twelve(12);

    // (imin,imax) = (0,5)
    success = Generate<2, 4>(0, 5, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_0,C_1,C_2,C_3,C_4,C_5) = (45,-154,214,-156,61,-10)/12
    // errorOrder = 4

    // (imin,imax) = (-1,4)
    success = Generate<2, 4>(-1, 4, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2,C_3,C_4) = (10,-15,-4,14,-6,1)/12
    // errorOrder = 4

    // (imin,imax) = (-2,3)
    success = Generate<2, 4>(-2, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1,C_2,C_3) = (-1,16,-30,16,-1,0)/12
    // errorOrder = 4

    // (imin,imax) = (-3,2)
    success = Generate<2, 4>(-3, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_{0},C_1,C_2) = (0,-1,16,-30,16,-1)/12
    // errorOrder = 4

    // (imin,imax) = (-4,1)
    success = Generate<2, 4>(-4, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-4},C_{-3},C_{-2},C_{-1},C_0,C_1) = (1,-6,14,-4,-15,10)/12
    // errorOrder = 4

    // (imin,imax) = (-5,0)
    success = Generate<2, 4>(-5, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = twelve * C[i];
        }
    }
    // (C_{-5},C_{-4},C_{-3},C_{-2},C_{-1},C_0) = (-10,61,-156,214,-154,45)/12
    // errorOrder = 4

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM3P1()
{
    bool success;
    std::array<Rational, 4> C;
    std::array<double, 4> dC;
    int errorOrder;

    // (imin,imax) = (0,3)
    success = Generate<3, 1>(0, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_0,C_1,C_2,C_3) = (-1,3,-3,1)
    // errorOrder = 1

    // (imin,imax) = (-1,2)
    success = Generate<3, 1>(-1, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2) = (-1,3,-3,1)
    // errorOrder = 1

    // (imin,imax) = (-2,1)
    success = Generate<3, 1>(-2, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1) = (-1,3,-3,1)
    // errorOrder = 1

    // (imin,imax) = (-3,0)
    success = Generate<3, 1>(-3, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_0) = (-1,3,-3,1)
    // errorOrder = 1

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM3P2()
{
    bool success;
    std::array<Rational, 5> C;
    std::array<double, 5> dC;
    int errorOrder;

    Rational two(2);

    // (imin,imax) = (0,4)
    success = Generate<3, 2>(0, 4, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = two * C[i];
        }
    }
    // (C_0,C_1,C_2,C_3,C_4) = (-5,18,-24,14,-3)/2
    // errorOrder = 2

    // (imin,imax) = (-1,3)
    success = Generate<3, 2>(-1, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = two * C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2,C_3) = (-3,10,-12,6,-1)/2
    // errorOrder = 2

    // (imin,imax) = (-2,2)
    success = Generate<3, 2>(-2, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = two * C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1,C_2) = ()/2
    // errorOrder = 2

    // (imin,imax) = (-3,1)
    success = Generate<3, 2>(-3, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = two * C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_0,C_1) = (1,-6,12,-10,3)/2
    // errorOrder = 2

    // (imin,imax) = (-4,0)
    success = Generate<3, 2>(-4, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = two * C[i];
        }
    }
    // (C_{-4},C_{-3},C_{-2},C_{-1},C_0) = (3,-14,24,-18,5)/2
    // errorOrder = 2

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM3P3()
{
    bool success;
    std::array<Rational, 6> C;
    std::array<double, 6> dC;
    int errorOrder;

    Rational four(4);

    // (imin,imax) = (0,5)
    success = Generate<3, 3>(0, 5, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = four * C[i];
        }
    }
    // (C_0,C_1,C_2,C_3,C_4,C_5) = (-17,71,-118,98,-41,7)/4
    // errorOrder = 3

    // (imin,imax) = (-1,4)
    success = Generate<3, 3>(-1, 4, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = four * C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2,C_3,C_4) = (-7,25,-34,22,-7,1)/4
    // errorOrder = 3

    // (imin,imax) = (-2,3)
    success = Generate<3, 3>(-2, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = four * C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1,C_2,C_3) = (-1,-1,10,-14,7,-1)/4
    // errorOrder = 3

    // (imin,imax) = (-3,2)
    success = Generate<3, 3>(-3, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = four * C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_0,C_1,C_2) = (1,-7,14,-10,1,1)/4
    // errorOrder = 3

    // (imin,imax) = (-4,1)
    success = Generate<3, 3>(-4, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = four * C[i];
        }
    }
    // (C_{-4},C_{-3},C_{-2},C_{-1},C_0,C_1) = (-1,7,-22,34,-25,7)/4
    // errorOrder = 3

    // (imin,imax) = (-5,0)
    success = Generate<3, 3>(-5, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = four * C[i];
        }
    }
    // (C_{-5},C_{-4},C_{-3},C_{-2},C_{-1},C_0) = (-7,41,-98,118,-71,17)/4
    // errorOrder = 3

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM3P4()
{
    bool success;
    std::array<Rational, 7> C;
    std::array<double, 7> dC;
    int errorOrder;

    Rational eight(8);

    // (imin,imax) = (0,6)
    success = Generate<3, 4>(0, 6, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = eight * C[i];
        }
    }
    // (C_0,C_1,C_2,C_3,C_4,C_5,C_6) = (-49,232,-461,496,-307,104,-15)/8
    // errorOrder = 4

    // (imin,imax) = (-1,5)
    success = Generate<3, 4>(-1, 5, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = eight * C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2,C_3,C_4,C_5) = (-15,56,-83,64,-29,8,-1)/8
    // errorOrder = 4

    // (imin,imax) = (-2,4)
    success = Generate<3, 4>(-2, 4, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = eight * C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1,C_2,C_3,C_4) = (-1,-8,35,-48,29,-8,1)/8
    // errorOrder = 4

    // (imin,imax) = (-3,3)
    success = Generate<3, 4>(-3, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = eight * C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_0,C_1,C_2,C_3) = (1,-8,13,0,-13,8,-1)/8
    // errorOrder = 4

    // (imin,imax) = (-4,2)
    success = Generate<3, 4>(-4, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = eight * C[i];
        }
    }
    // (C_{-4},C_{-3},C_{-2},C_{-1},C_0,C_1,C_2) = (-1,8,-29,48,-35,8,1)/8
    // errorOrder = 4

    // (imin,imax) = (-5,1)
    success = Generate<3, 4>(-5, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = eight * C[i];
        }
    }
    // (C_{-5},C_{-4},C_{-3},C_{-2},C_{-1},C_0,C_1) = (1,-8,29,-64,83,-56,15)/8
    // errorOrder = 4

    // (imin,imax) = (-6,0)
    success = Generate<3, 4>(-6, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = eight * C[i];
        }
    }
    // (C_{-6},C_{-5},C_{-4},C_{-3},C_{-2},C_{-1},C_0) = (15,-104,307,-496,461,-232,49)/8
    // errorOrder = 4

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM4P1()
{
    bool success;
    std::array<Rational, 5> C;
    std::array<double, 5> dC;
    int errorOrder;

    // (imin,imax) = (0,4)
    success = Generate<4, 1>(0, 4, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_0,C_1,C_2,C_3,C_4) = (1,-4,6,-4,1)
    // errorOrder = 1

    // (imin,imax) = (-1,3)
    success = Generate<4, 1>(-1, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2,C_3) = (1,-4,6,-4,1)
    // errorOrder = 1

    // (imin,imax) = (-2,2)
    success = Generate<4, 1>(-2, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1,C_2) = (1,-4,6,-4,1)
    // errorOrder = 2

    // (imin,imax) = (-3,1)
    success = Generate<4, 1>(-3, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_0,C_1) = (1,-4,6,-4,1)
    // errorOrder = 1

    // (imin,imax) = (-4,0)
    success = Generate<4, 1>(-4, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-4},C_{-3},C_{-2},C_{-1},C_0) = (1,-4,6,-4,1)
    // errorOrder = 1

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM4P2()
{
    bool success;
    std::array<Rational, 6> C;
    std::array<double, 6> dC;
    int errorOrder;

    // (imin,imax) = (0,5)
    success = Generate<4, 2>(0, 5, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_0,C_1,C_2,C_3,C_4,C_5) = (3,-14,26,-24,11,-2)
    // errorOrder = 2

    // (imin,imax) = (-1,4)
    success = Generate<4, 2>(-1, 4, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2,C_3,C_4) = (2,-9,16,-14,6,-1)
    // errorOrder = 2

    // (imin,imax) = (-2,3)
    success = Generate<4, 2>(-2, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1,C_2,C_3) = (1,-4,6,-4,1,0)
    // errorOrder = 2

    // (imin,imax) = (-3,2)
    success = Generate<4, 2>(-3, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_0,C_1,C_2) = (0,1,-4,6,-4,1)
    // errorOrder = 2

    // (imin,imax) = (-4,1)
    success = Generate<4, 2>(-4, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-4},C_{-3},C_{-2},C_{-1},C_0,C_1) = (-1,6,-14,16,-9,2)
    // errorOrder = 2

    // (imin,imax) = (-5,0)
    success = Generate<4, 2>(-5, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            dC[i] = C[i];
        }
    }
    // (C_{-5},C_{-4},C_{-3},C_{-2},C_{-1},C_0) = (-2,11,-24,26,-14,3)
    // errorOrder = 2

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM4P3()
{
    bool success;
    std::array<Rational, 7> C;
    std::array<double, 7> dC;
    int errorOrder;

    Rational six(6);

    // (imin,imax) = (0,6)
    success = Generate<4, 3>(0, 6, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_0,C_1,C_2,C_3,C_4,C_5,C_6) = (35,-186,411,-484,321,-114,17)/6
    // errorOrder = 3

    // (imin,imax) = (-1,5)
    success = Generate<4, 3>(-1, 5, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2,C_3,C_4,C_5) = (17,-84,171,-184,111,-36,5)/6
    // errorOrder = 3

    // (imin,imax) = (-2,4)
    success = Generate<4, 3>(-2, 4, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1,C_2,C_3,C_4) = (5,-18,21,-4,-9,6,-1)/6
    // errorOrder = 3

    // (imin,imax) = (-3,3)
    success = Generate<4, 3>(-3, 3, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_0,C_1,C_2,C_3) = (-1,12,-39,56,-39,12,-1)/6
    // errorOrder = 4

    // (imin,imax) = (-4,2)
    success = Generate<4, 3>(-4, 2, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_{-4},C_{-3},C_{-2},C_{-1},C_0,C_1,C_2) = (-1,6,-9,-4,21,-18,5)/6
    // errorOrder = 3

    // (imin,imax) = (-5,1)
    success = Generate<4, 3>(-5, 1, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_{-5},C_{-4},C_{-3},C_{-2},C_{-1},C_0,C_1) = (5,-36,111,-184,171,-84,17)/6
    // errorOrder = 3

    // (imin,imax) = (-6,0)
    success = Generate<4, 3>(-6, 0, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_{-6},C_{-5},C_{-4},C_{-3},C_{-2},C_{-1},C_0) = (17,-114,321,-484,411,-186,35)/6
    // errorOrder = 3

    int stophere;
    stophere = 0;
}

void FiniteDifferencesConsole::GenerateM4P4()
{
    bool success;
    std::array<Rational, 8> C;
    std::array<double, 8> dC;
    int errorOrder;

    Rational six(6);

#if 0
    // (imin,imax) = (0,7)
    success = Generate<4, 4>(0, 7, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 8; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_0,C_1,C_2,C_3,C_4,C_5,C_6,C_7)
    // = (56,-333,852,-1219,1056,-555,164,-21)/6
    // errorOrder = 4

    // (imin,imax) = (-1,6)
    success = Generate<4, 4>(-1, 6, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 8; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_{-1},C_0,C_1,C_2,C_3,C_4,C_5,C_6)
    // = (21,-112,255,-324,251,-120,33,-4)/6
    // errorOrder = 4

    // (imin,imax) = (-2,5)
    success = Generate<4, 4>(-2, 5, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 8; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_{-2},C_{-1},C_0,C_1,C_2,C_3,C_4,C_5)
    // = (4,-11,0,31,-44,27,-8,1)/6
    // errorOrder = 4
#endif

    // (imin,imax) = (-3,4)
    success = Generate<4, 4>(-3, 4, C, errorOrder);
    if (success)
    {
        for (size_t i = 0; i < 8; ++i)
        {
            dC[i] = six * C[i];
        }
    }
    // (C_{-3},C_{-2},C_{-1},C_0,C_1,C_2,C_3,C_4)
    // = (-1,12,-39,56,-39,12,-1,0)/6
    // errorOrder = 4

    int stophere;
    stophere = 0;
}
