// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "DisjointIntervalsRectanglesConsole.h"
#include <Mathematics/DisjointIntervals.h>
#include <Mathematics/DisjointRectangles.h>
using namespace gte;

DisjointIntervalsRectanglesConsole::DisjointIntervalsRectanglesConsole(Parameters& parameters)
    :
    Console(parameters)
{
}

void DisjointIntervalsRectanglesConsole::Execute()
{
    float xmin, xmax, ymin, ymax;
    DisjointIntervals<float> intervalSet;

    DisjointIntervals<float> S1, S2;
    S1.Insert(1.0f, 3.0f);
    S1.Insert(4.0f, 9.0f);
    S1.Insert(10.0f, 12.0f);
    S1.Insert(16.0f, 17.0f);
    S2.Insert(0.0f, 2.0f);
    S2.Insert(6.0f, 7.0f);
    S2.Insert(8.0f, 11.0f);
    S2.Insert(13.0f, 14.0f);
    S2.Insert(15.0f, 18.0f);

    // union
    DisjointIntervals<float> S1uS2 = S1 | S2;
    LogAssert(S1uS2.GetNumIntervals() == 4, "Incorrect number of union intervals.");
    S1uS2.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 0.0f && xmax == 3.0f, "Incorrect union interval.");
    S1uS2.GetInterval(1, xmin, xmax);
    LogAssert(xmin == 4.0f && xmax == 12.0f, "Incorrect union interval.");
    S1uS2.GetInterval(2, xmin, xmax);
    LogAssert(xmin == 13.0f && xmax == 14.0f, "Incorrect union interval.");
    S1uS2.GetInterval(3, xmin, xmax);
    LogAssert(xmin == 15.0f && xmax == 18.0f, "Incorrect union interval.");

    // intersection
    DisjointIntervals<float> S1iS2 = S1 & S2;
    LogAssert(S1iS2.GetNumIntervals() == 5, "Incorrect number of intersection intervals.");
    S1iS2.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 1.0f && xmax == 2.0f, "Incorrect intersection interval.");
    S1iS2.GetInterval(1, xmin, xmax);
    LogAssert(xmin == 6.0f && xmax == 7.0f, "Incorrect intersection interval.");
    S1iS2.GetInterval(2, xmin, xmax);
    LogAssert(xmin == 8.0f && xmax == 9.0f, "Incorrect intersection interval.");
    S1iS2.GetInterval(3, xmin, xmax);
    LogAssert(xmin == 10.0f && xmax == 11.0f, "Incorrect intersection interval.");
    S1iS2.GetInterval(4, xmin, xmax);
    LogAssert(xmin == 16.0f && xmax == 17.0f, "Incorrect intersection interval.");

    // difference
    DisjointIntervals<float> S1dS2 = S1 - S2;
    LogAssert(S1dS2.GetNumIntervals() == 4, "Incorrect number of difference intervals.");
    S1dS2.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 2.0f && xmax == 3.0f, "Incorrect difference interval.");
    S1dS2.GetInterval(1, xmin, xmax);
    LogAssert(xmin == 4.0f && xmax == 6.0f, "Incorrect difference interval.");
    S1dS2.GetInterval(2, xmin, xmax);
    LogAssert(xmin == 7.0f && xmax == 8.0f, "Incorrect difference interval.");
    S1dS2.GetInterval(3, xmin, xmax);
    LogAssert(xmin == 11.0f && xmax == 12.0f, "Incorrect difference interval.");

    // exclusive-or
    DisjointIntervals<float> S1xS2 = S1 ^ S2;
    LogAssert(S1xS2.GetNumIntervals() == 9, "Incorrect number of xor intervals.");
    S1xS2.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 0.0f && xmax == 1.0f, "Incorrect xor interval.");
    S1xS2.GetInterval(1, xmin, xmax);
    LogAssert(xmin == 2.0f && xmax == 3.0f, "Incorrect xor interval.");
    S1xS2.GetInterval(2, xmin, xmax);
    LogAssert(xmin == 4.0f && xmax == 6.0f, "Incorrect xor interval.");
    S1xS2.GetInterval(3, xmin, xmax);
    LogAssert(xmin == 7.0f && xmax == 8.0f, "Incorrect xor interval.");
    S1xS2.GetInterval(4, xmin, xmax);
    LogAssert(xmin == 9.0f && xmax == 10.0f, "Incorrect xor interval.");
    S1xS2.GetInterval(5, xmin, xmax);
    LogAssert(xmin == 11.0f && xmax == 12.0f, "Incorrect xor interval.");
    S1xS2.GetInterval(6, xmin, xmax);
    LogAssert(xmin == 13.0f && xmax == 14.0f, "Incorrect xor interval.");
    S1xS2.GetInterval(7, xmin, xmax);
    LogAssert(xmin == 15.0f && xmax == 16.0f, "Incorrect xor interval.");
    S1xS2.GetInterval(8, xmin, xmax);
    LogAssert(xmin == 17.0f && xmax == 18.0f, "Incorrect xor interval.");

    DisjointRectangles<float> R1, R2;
    R1.Insert(0.0f, 2.0f, 0.0f, 2.0f);
    R2.Insert(1.0f, 3.0f, 1.0f, 3.0f);

    // union
    DisjointRectangles<float> R1uR2 = R1 | R2;
    LogAssert(R1uR2.GetNumRectangles() == 3, "Incorrect number of union rectangles.");
    LogAssert(R1uR2.GetNumStrips() == 3, "Incorrect number of union strips.");
    R1uR2.GetStrip(0, ymin, ymax, intervalSet);
    LogAssert(intervalSet.GetNumIntervals() == 1, "Incorrect number of union intervals.");
    LogAssert(ymin == 0.0f && ymax == 1.0f, "Incorrect union strip extremes.");
    intervalSet.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 0.0f && xmax == 2.0f, "Incorrect union interval.");
    R1uR2.GetStrip(1, ymin, ymax, intervalSet);
    LogAssert(intervalSet.GetNumIntervals() == 1, "Incorrect number of union intervals.");
    LogAssert(ymin == 1.0f && ymax == 2.0f, "Incorrect union strip extremes.");
    intervalSet.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 0.0f && xmax == 3.0f, "Incorrect union interval.");
    R1uR2.GetStrip(2, ymin, ymax, intervalSet);
    LogAssert(intervalSet.GetNumIntervals() == 1, "Incorrect number of union intervals.");
    LogAssert(ymin == 2.0f && ymax == 3.0f, "Incorrect union strip extremes.");
    intervalSet.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 1.0f && xmax == 3.0f, "Incorrect union interval.");

    // intersection
    DisjointRectangles<float> R1iR2 = R1 & R2;
    LogAssert(R1iR2.GetNumRectangles() == 1, "Incorrect number of intersection rectangles.");
    LogAssert(R1iR2.GetNumStrips() == 1, "Incorrect number of intersection strips.");
    R1iR2.GetStrip(0, ymin, ymax, intervalSet);
    LogAssert(intervalSet.GetNumIntervals() == 1, "Incorrect number of intersection intervals.");
    LogAssert(ymin == 1.0f && ymax == 2.0f, "Incorrect intersection strip extremes.");
    intervalSet.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 1.0f && xmax == 2.0f, "Incorrect intersection interval.");

    // difference
    DisjointRectangles<float> R1dR2 = R1 - R2;
    LogAssert(R1dR2.GetNumRectangles() == 2, "Incorrect number of difference rectangles.");
    LogAssert(R1dR2.GetNumStrips() == 2, "Incorrect number of difference strips.");
    R1dR2.GetStrip(0, ymin, ymax, intervalSet);
    LogAssert(intervalSet.GetNumIntervals() == 1, "Incorrect number of difference intervals.");
    LogAssert(ymin == 0.0f && ymax == 1.0f, "Incorrect difference strip extremes.");
    intervalSet.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 0.0f && xmax == 2.0f, "Incorrect difference interval.");
    R1dR2.GetStrip(1, ymin, ymax, intervalSet);
    LogAssert(intervalSet.GetNumIntervals() == 1, "Incorrect number of difference intervals.");
    LogAssert(ymin == 1.0f && ymax == 2.0f, "Incorrect difference strip extremes.");
    intervalSet.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 0.0f && xmax == 1.0f, "Incorrect difference interval.");

    // exclusive-or
    DisjointRectangles<float> R1xR2 = R1 ^ R2;
    LogAssert(R1xR2.GetNumRectangles() == 4, "Incorrect number of xor rectangles.");
    LogAssert(R1xR2.GetNumStrips() == 3, "Incorrect number of xor strips.");
    R1xR2.GetStrip(0, ymin, ymax, intervalSet);
    LogAssert(intervalSet.GetNumIntervals() == 1, "Incorrect number of xor intervals.");
    LogAssert(ymin == 0.0f && ymax == 1.0f, "Incorrect xor strip extremes.");
    intervalSet.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 0.0f && xmax == 2.0f, "Incorrect xor interval.");
    R1xR2.GetStrip(1, ymin, ymax, intervalSet);
    LogAssert(intervalSet.GetNumIntervals() == 2, "Incorrect number of xor intervals.");
    LogAssert(ymin == 1.0f && ymax == 2.0f, "Incorrect xor strip extremes.");
    intervalSet.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 0.0f && xmax == 1.0f, "Incorrect xor interval.");
    intervalSet.GetInterval(1, xmin, xmax);
    LogAssert(xmin == 2.0f && xmax == 3.0f, "Incorrect xor interval.");
    R1xR2.GetStrip(2, ymin, ymax, intervalSet);
    LogAssert(intervalSet.GetNumIntervals() == 1, "Incorrect number of xor intervals.");
    LogAssert(ymin == 2.0f && ymax == 3.0f, "Incorrect xor strip extremes.");
    intervalSet.GetInterval(0, xmin, xmax);
    LogAssert(xmin == 1.0f && xmax == 3.0f, "Incorrect xor interval.");
}
