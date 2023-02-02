// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Console.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/DistSegmentSegment.h>
#include <fstream>
using namespace gte;

class DistanceSegments3Console : public Console
{
public:
    DistanceSegments3Console(Parameters& parameters);

    virtual void Execute() override;

private:
    enum { PERF_SUNDAY, PERF_ROBUST, PERF_RATIONAL };
    typedef BSRational<UIntegerFP32<128>> Rational;
    typedef DCPQuery<double, Segment<3, double>, Segment<3, double>> RobustQuery;
    typedef DCPQuery<Rational, Segment<3, Rational>, Segment<3, Rational>> RationalQuery;

    bool SetEnvironment();

    // The function dist3D_Segment_to_Segment is from Dan Sunday's website:
    //   http://geomalgorithms.com/a07-_distance.html
    // with some modifications. The inputs of type Segment were replaced by
    // point pairs of type Vector3<double> and the algebraic operator calls
    // were replaced accordingly. The distance is now returned (with other
    // parameters) as arguments to the function.  The SMALL_NUM macro was
    // replaced by a 'const' declaration.  The modified code computes the
    // closest points. See the revised document (as of 2014/11/05)
    //   https://www.geometrictools.com/Documentation/DistanceLine3Line3.pdf
    // that describes an algorithm that is robust, particularly for nearly
    // segments, and that uses floating-point arithmetic.  An example in this
    // PDF shows that there is a problem with the logic of Sunday's algorithm
    // when D < SMALL_NUM and the search is started on the s=0 edge.
    // Specifically, the closest points are not found correctly--the closest
    // point on the first segment occurs when s=1. No contact information is
    // at his website, so we are unable to report the problem to him.
    void dist3D_Segment_to_Segment(
        Vector3<double> const& P0, Vector3<double> const& P1,
        Vector3<double> const& Q0, Vector3<double> const& Q1,
        double& sqrDistance, double& s, double& t, Vector3<double> closest[2]);

    template <int32_t N>
    void LoadInput(bool testNonparallel, uint32_t numInputs,
        Segment<N, double>* segment);

    void CPUAccuracyTest(bool compareUsingExact, bool testNonparallel);
    void CPUPerformanceTest(int32_t select, bool testNonparallel);
    void GPUAccuracyTest(bool getClosest, bool testNonparallel);
    void GPUPerformanceTest(bool getClosest, bool testNonparallel);

    std::ofstream mOutput;
};
