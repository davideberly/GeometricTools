// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "DistanceSegments3Console.h"
#include <Applications/Timer.h>
#include <iostream>

DistanceSegments3Console::DistanceSegments3Console(Parameters& parameters)
    :
    Console(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }
}

void DistanceSegments3Console::Execute()
{
    mOutput.open("DS3Output.txt");

    // The experiments were run on an Intel Core i7-6700 CPU @ 3.40 GHz and an
    // NVIDIA GeForce GTX 1080.  The CPU runs are single-threaded.  The times
    // are for the Release build run without a debugger attached.  The GPU
    // tests use the robust algorithm, so the times must be compared to those
    // of the CPU PERF_ROBUST.

    // max error02 = 4.44089e-16 at (x,y) = (346,1)
    // max error12 = 4.44089e-16 at (x,y) = (346,1)
    // max error01 = 6.66134e-16 at (x,y) = (520,288)
    mOutput << "CPUAccuracyTest(true, true)" << std::endl;
    CPUAccuracyTest(true, true);
    mOutput << std::endl;

    // max error02 = 3.52850e-07 at (x,y) = (362,283)
    // max error12 = 4.17519e-08 at (x,y) = (994,186)
    // max error01 = 3.51795e-07 at (x,y) = (722,362)
    mOutput << "CPUAccuracyTest(true, false)" << std::endl;
    CPUAccuracyTest(true, false);
    mOutput << std::endl;

    // max error01 = 6.66134e-16 at (x,y) = (520,288)
    mOutput << "CPUAccuracyTest(false, true)" << std::endl;
    CPUAccuracyTest(false, true);
    mOutput << std::endl;

    // max error01 = 1.09974e-06 at (x,y) = (1024,569)
    mOutput << "CPUAccuracyTest(false, false)" << std::endl;
    CPUAccuracyTest(false, false);
    mOutput << std::endl;

    // seconds = 0.193, numQueries = 8386560, seconds per query = 2.3013e-08
    mOutput << "CPUPerformanceTest(PERF_SUNDAY, true)" << std::endl;
    CPUPerformanceTest(PERF_SUNDAY, true);
    mOutput << std::endl;

    // seconds = 0.176, numQueries = 8386560, seconds per query = 2.0986e-08
    mOutput << "CPUPerformanceTest(PERF_SUNDAY, false)" << std::endl;
    CPUPerformanceTest(PERF_SUNDAY, false);
    mOutput << std::endl;

    // seconds = 0.338, numQueries = 8386560, seconds per query = 4.03026e-08
    mOutput << "CPUPerformanceTest(PERF_ROBUST, true)" << std::endl;
    CPUPerformanceTest(PERF_ROBUST, true);
    mOutput << std::endl;

    // seconds = 0.348, numQueries = 8386560, seconds per query = 4.1495e-08
    mOutput << "CPUPerformanceTest(PERF_ROBUST, false)" << std::endl;
    CPUPerformanceTest(PERF_ROBUST, false);
    mOutput << std::endl;

    // seconds = 7.29, numQueries = 523776, seconds per query = 1.39182e-05
    mOutput << "CPUPerformanceTest(PERF_RATIONAL, true)" << std::endl;
    CPUPerformanceTest(PERF_RATIONAL, true);
    mOutput << std::endl;

    // seconds = 2.926, numQueries = 523776, seconds per query = 5.58636e-06
    mOutput << "CPUPerformanceTest(PERF_RATIONAL, false)" << std::endl;
    CPUPerformanceTest(PERF_RATIONAL, false);
    mOutput << std::endl;

    // DX11,   max error = 0 at (x,y) = (0,0)
    // OpenGL, max error = 6.66134e-16 at (x,y) = (116,79)
    mOutput << "GPUAccuracyTest(true, true)" << std::endl;
    GPUAccuracyTest(true, true);
    mOutput << std::endl;

    // DX11,   max error = 0 at (x,y) = (0,0)
    // OpenGL, max error = 2.95232e-08 at (x,y) = (931,880)
    mOutput << "GPUAccuracyTest(true, false)" << std::endl;
    GPUAccuracyTest(true, false);
    mOutput << std::endl;

    // DX11,   max error = 0 at (x,y) = (0,0)
    // OpenGL, max error = 6.66134e-16 at (x,y) = (116,79)
    mOutput << "GPUAccuracyTest(false, true)" << std::endl;
    GPUAccuracyTest(false, true);
    mOutput << std::endl;

    // DX11,   max error = 0 at (x,y) = (0,0)
    // OpenGL, max error = 2.95232e-08 at (x,y) = (931,880)
    mOutput << "GPUAccuracyTest(false, false)" << std::endl;
    GPUAccuracyTest(false, false);
    mOutput << std::endl;

    // DX11,   seconds = 0.312, numQueries = 10485760, seconds per query = 2.97546e-08
    // OpenGL, seconds = 0.394, numQueries = 10485760, seconds per query = 3.75748e-08
    mOutput << "GPUPerformanceTest(true, true)" << std::endl;
    GPUPerformanceTest(true, true);
    mOutput << std::endl;

    // DX11,   seconds = 0.308, numQueries = 10485760, seconds per query = 2.93732e-08
    // OpenGL, seconds = 0.398, numQueries = 10485760, seconds per query = 3.79562e-08
    mOutput << "GPUPerformanceTest(true, false)" << std::endl;
    GPUPerformanceTest(true, false);
    mOutput << std::endl;

    // DX11,   seconds = 0.127, numQueries = 10485760, seconds per query = 1.21117e-08
    // OpenGL, seconds = 0.172, numQueries = 10485760, seconds per query = 1.64032e-08
    mOutput << "GPUPerformanceTest(false, true)" << std::endl;
    GPUPerformanceTest(false, true);
    mOutput << std::endl;

    // DX11,   seconds = 0.122, numQueries = 10485760, seconds per query = 1.16348e-08
    // OpenGL, seconds = 0.169, numQueries = 10485760, seconds per query = 1.61171e-08
    mOutput << "GPUPerformanceTest(false, false)" << std::endl;
    GPUPerformanceTest(false, false);
    mOutput << std::endl;

    mOutput.close();
}

bool DistanceSegments3Console::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Distance/DistanceSegments3/Data/");
    mEnvironment.Insert(path + "/Samples/Distance/DistanceSegments3/Shaders/");

    std::vector<std::string> inputs =
    {
        "InputNonparallel.binary",
        "InputParallel.binary",
        mEngine->GetShaderName("DistanceSeg3Seg3.cs")
    };

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    return true;
}

void DistanceSegments3Console::dist3D_Segment_to_Segment(
    Vector3<double> const& P0, Vector3<double> const& P1,
    Vector3<double> const& Q0, Vector3<double> const& Q1,
    double& sqrDistance, double& s, double& t, Vector3<double> closest[2])
{
    double const SMALL_NUM = 0.00000001;
    Vector3<double>   u = P1 - P0;
    Vector3<double>   v = Q1 - Q0;
    Vector3<double>   w = P0 - Q0;
    double    a = Dot(u, u);         // always >= 0
    double    b = Dot(u, v);
    double    c = Dot(v, v);         // always >= 0
    double    d = Dot(u, w);
    double    e = Dot(v, w);
    double    D = a*c - b*b;        // always >= 0
    double    sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
    double    tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

    // compute the line parameters of the two closest points
    if (D < SMALL_NUM) { // the lines are almost parallel
        sN = 0.0;         // force using point P0 on segment S1
        sD = 1.0;         // to prevent possible division by 0.0 later
        tN = e;
        tD = c;
    }
    else {                 // get the closest points on the infinite lines
        sN = (b*e - c*d);
        tN = (a*e - b*d);
        if (sN < 0.0) {        // sc < 0 => the s=0 edge is visible
            sN = 0.0;
            tN = e;
            tD = c;
        }
        else if (sN > sD) {  // sc > 1  => the s=1 edge is visible
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    if (tN < 0.0) {            // tc < 0 => the t=0 edge is visible
        tN = 0.0;
        // recompute sc for this edge
        if (-d < 0.0)
            sN = 0.0;
        else if (-d > a)
            sN = sD;
        else {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD) {      // tc > 1  => the t=1 edge is visible
        tN = tD;
        // recompute sc for this edge
        if ((-d + b) < 0.0)
            sN = 0;
        else if ((-d + b) > a)
            sN = sD;
        else {
            sN = (-d + b);
            sD = a;
        }
    }
    // finally do the division to get sc and tc
    sc = (std::fabs(sN) < SMALL_NUM ? 0.0 : sN / sD);
    tc = (std::fabs(tN) < SMALL_NUM ? 0.0 : tN / tD);

    // get the difference of the two closest points
    s = sc;
    t = tc;
    closest[0] = (1.0 - sc) * P0 + sc * P1;
    closest[1] = (1.0 - tc) * Q0 + tc * Q1;
    Vector3<double> diff = closest[0] - closest[1];
    sqrDistance = Dot(diff, diff);
}

template <int32_t N>
void DistanceSegments3Console::LoadInput(bool testNonparallel, uint32_t numInputs, Segment<N, double>* segment)
{
    int32_t numChannels = N;

    if (testNonparallel)
    {
        std::string path = mEnvironment.GetPath("InputNonparallel.binary");
        std::ifstream input(path, std::ios::binary);
        for (uint32_t i = 0; i < numInputs; ++i)
        {
            for (int32_t j = 0; j < 3; ++j)
            {
                input.read((char*)& segment[i].p[0][j], sizeof(double));
                input.read((char*)& segment[i].p[1][j], sizeof(double));
            }
            if (numChannels == 4)
            {
                segment[i].p[0][3] = 1.0;
                segment[i].p[1][3] = 1.0;
            }
        }
        input.close();
    }
    else
    {
        std::string path = mEnvironment.GetPath("InputParallel.binary");
        std::ifstream input(path, std::ios::binary);
        for (uint32_t i = 0; i < numInputs; ++i)
        {
            input.read((char*)& segment[i].p[0][0], sizeof(double));
            input.read((char*)& segment[i].p[0][1], sizeof(double));
            input.read((char*)& segment[i].p[0][2], sizeof(double));
            input.read((char*)& segment[i].p[1][0], sizeof(double));
            input.read((char*)& segment[i].p[1][1], sizeof(double));
            input.read((char*)& segment[i].p[1][2], sizeof(double));
            if (numChannels == 4)
            {
                segment[i].p[0][3] = 1.0;
                segment[i].p[1][3] = 1.0;
            }
        }
        input.close();
    }
}

void DistanceSegments3Console::CPUAccuracyTest(bool compareUsingExact, bool testNonparallel)
{
    // NOTE: When comparing to exact arithmetic results, the number of inputs
    // needs to be smaller because the exact algorithm is expensive to
    // compute. In this case the maximum errors are all small (4e-16).
    // However, when not comparing to exact results, maxError01 is on the
    // order of 1e-4. The pair of segments that generate the maximum error
    // shows that the logic of dist3D_Segment_to_Segment when segments are
    // nearly parallel is not correct.
    uint32_t const numInputs = (compareUsingExact ? 1024 : 4096);
    uint32_t const numBlocks = 16;
    std::vector<Segment<3, double>> segment(numInputs);

    LoadInput(testNonparallel, numInputs, segment.data());

    double maxError01 = 0.0, maxError02 = 0.0, maxError12 = 0.0, error;
    uint32_t xmax01 = 0, ymax01 = 0;
    uint32_t xmax02 = 0, ymax02 = 0;
    uint32_t xmax12 = 0, ymax12 = 0;

    for (uint32_t y = 0; y < numInputs; ++y)
    {
        if ((y % numBlocks) == 0)
        {
            std::cout << "y = " << y << std::endl;
        }

        Vector3<double> Q0 = segment[y].p[0];
        Vector3<double> Q1 = segment[y].p[1];

        for (uint32_t x = y + 1; x < numInputs; ++x)
        {
            Vector3<double> P0 = segment[x].p[0];
            Vector3<double> P1 = segment[x].p[1];

            // Sunday's query
            double sqrDistance0, s0, t0;
            Vector3<double> closest0[2];
            dist3D_Segment_to_Segment(P0, P1, Q0, Q1, sqrDistance0, s0, t0, closest0);
            double distance0 = std::sqrt(sqrDistance0);

            // robust query
            RobustQuery query1{};
            auto result1 = query1.ComputeRobust(P0, P1, Q0, Q1);
            double distance1 = result1.distance;

            if (compareUsingExact)
            {
                // rational query
                Vector3<Rational> RP0{ P0[0], P0[1], P0[2] };
                Vector3<Rational> RP1{ P1[0], P1[1], P1[2] };
                Vector3<Rational> RQ0{ Q0[0], Q0[1], Q0[2] };
                Vector3<Rational> RQ1{ Q1[0], Q1[1], Q1[2] };
                RationalQuery query2{};
                auto result2 = query2(RP0, RP1, RQ0, RQ1);
                double distance2 = std::sqrt((double)result2.sqrDistance);

                error = std::fabs(distance0 - distance2);
                if (error > maxError02)
                {
                    maxError02 = error;
                    xmax02 = x;
                    ymax02 = y;
                }

                error = std::fabs(distance1 - distance2);
                if (error > maxError12)
                {
                    maxError12 = error;
                    xmax12 = x;
                    ymax12 = y;
                }
            }

            error = std::fabs(distance0 - distance1);
            if (error > maxError01)
            {
                maxError01 = error;
                xmax01 = x;
                ymax01 = y;
            }
        }
    }

    if (compareUsingExact)
    {
        std::cout << "max error02 = " << maxError02 << std::endl;
        std::cout << "x, y = " << xmax02 << " " << ymax02 << std::endl;
        std::cout << "max error12 = " << maxError12 << std::endl;
        std::cout << "x, y = " << xmax12 << " " << ymax12 << std::endl;
        mOutput << "max error02 = " << maxError02 << std::endl;
        mOutput << "x, y = " << xmax02 << " " << ymax02 << std::endl;
        mOutput << "max error12 = " << maxError12 << std::endl;
        mOutput << "x, y = " << xmax12 << " " << ymax12 << std::endl;
    }
    std::cout << "max error01 = " << maxError01 << std::endl;
    std::cout << "x, y = " << xmax01 << " " << ymax01 << std::endl;
    mOutput << "max error01 = " << maxError01 << std::endl;
    mOutput << "x, y = " << xmax01 << " " << ymax01 << std::endl;
}

void DistanceSegments3Console::CPUPerformanceTest(int32_t select, bool testNonparallel)
{
    uint32_t const numInputs = (select == PERF_RATIONAL ? 1024 : 4096);
    std::vector<Segment<3, double>> segment(numInputs);
    uint32_t numQueries;

    LoadInput(testNonparallel, numInputs, segment.data());

    Timer timer;

    if (select == PERF_SUNDAY)
    {
        double sqrDistance0, s0, t0;
        Vector3<double> closest0[2];
        numQueries = 0;
        for (uint32_t y = 0; y < numInputs; ++y)
        {
            for (uint32_t x = y + 1; x < numInputs; ++x, ++numQueries)
            {
                dist3D_Segment_to_Segment(
                    segment[x].p[0], segment[x].p[1],
                    segment[y].p[0], segment[y].p[1],
                    sqrDistance0, s0, t0, closest0);
            }
        }
    }
    else if (select == PERF_ROBUST)
    {
        RobustQuery query{};
        RobustQuery::Result result{};
        numQueries = 0;
        for (uint32_t y = 0; y < numInputs; ++y)
        {
            for (uint32_t x = y + 1; x < numInputs; ++x, ++numQueries)
            {
                result = query.ComputeRobust(segment[x], segment[y]);
            }
        }
    }
    else  // select == PERF_RATIONAL
    {
        RationalQuery query{};
        RationalQuery::Result result{};
        Vector3<Rational> RP0, RP1, RQ0, RQ1;
        numQueries = 0;
        for (uint32_t y = 0; y < numInputs; ++y)
        {
            for (int32_t i = 0; i < 3; ++i)
            {
                RQ0[i] = segment[y].p[0][i];
                RQ1[i] = segment[y].p[1][i];
            }
            for (uint32_t x = y + 1; x < numInputs; ++x, ++numQueries)
            {
                for (int32_t i = 0; i < 3; ++i)
                {
                    RP0[i] = segment[x].p[0][i];
                    RP1[i] = segment[x].p[1][i];
                }
                result = query(RP0, RP1, RQ0, RQ1);
            }
        }
    }

    double seconds = timer.GetSeconds();
    double secondsPerQuery = seconds / static_cast<double>(numQueries);
    std::cout
        << "seconds = " << seconds << ", "
        << "numQueries = " << numQueries << ", "
        << "seconds per query = " << secondsPerQuery << std::endl;
    mOutput
        << "seconds = " << seconds << ", "
        << "numQueries = " << numQueries << ", "
        << "seconds per query = " << secondsPerQuery << std::endl;
}

void DistanceSegments3Console::GPUAccuracyTest(bool getClosest, bool testNonparallel)
{
    uint32_t const numInputs = 4096;
    uint32_t const blockSize = 1024;
    uint32_t const numBlocks = numInputs / blockSize;
    uint32_t const numThreads = 8;
    uint32_t const numGroups = blockSize / numThreads;

    mProgramFactory->defines.Set("NUM_X_THREADS", numThreads);
    mProgramFactory->defines.Set("NUM_Y_THREADS", numThreads);
    mProgramFactory->defines.Set("BLOCK_SIZE", blockSize);
    mProgramFactory->defines.Set("REAL", "double");
#if defined(GTE_USE_OPENGL)
    mProgramFactory->defines.Set("VECREAL", "dvec4");
#endif
#if defined(GTE_USE_DIRECTX)
    mProgramFactory->defines.Set("VECREAL", "double4");
#endif
    mProgramFactory->defines.Set("GET_CLOSEST", (getClosest ? 1 : 0));

    auto cprogram = mProgramFactory->CreateFromFile(
        mEnvironment.GetPath(mEngine->GetShaderName("DistanceSeg3Seg3.cs")));
    auto const& cshader = cprogram->GetComputeShader();
    auto block = std::make_shared<ConstantBuffer>(2 * sizeof(uint32_t), true);
    cshader->Set("Block", block);
    auto* origin = block->Get<uint32_t>();

    auto input = std::make_shared<StructuredBuffer>(numInputs, sizeof(Segment<4, double>));
    input->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    cshader->Set("inSegment", input);
    auto* segment = input->Get<Segment<4, double>>();

    LoadInput(testNonparallel, numInputs, segment);

    std::shared_ptr<StructuredBuffer> output;
    double maxError = 0.0;
    int32_t xmax = 0, ymax = 0;
    if (getClosest)
    {
        // GLSL wants closest[] to be aligned on a dvec4 boundary, so
        // parameter[2] is padding.
        struct Result
        {
            double sqrDistance;
            double parameter[3];
            Vector4<double> closest[2];
        };

        output = std::make_shared<StructuredBuffer>(blockSize * blockSize, sizeof(Result));
        output->SetUsage(Resource::Usage::SHADER_OUTPUT);
        output->SetCopy(Resource::Copy::STAGING_TO_CPU);
        Result* gpuResult = output->Get<Result>();
        cshader->Set("outResult", output);

        for (uint32_t y = 0, i = 0; y < numBlocks; ++y)
        {
            std::cout << "block = " << y << std::endl;
            origin[1] = y * blockSize;
            for (uint32_t x = y; x < numBlocks; ++x, ++i)
            {
                origin[0] = x * blockSize;
                mEngine->Update(block);
                mEngine->Execute(cprogram, numGroups, numGroups, 1);
                mEngine->CopyGpuToCpu(output);

                for (uint32_t r = 0; r < blockSize; ++r)
                {
                    int32_t sy = origin[1] + r;
                    Vector3<double> Q0 = HProject(segment[sy].p[0]);
                    Vector3<double> Q1 = HProject(segment[sy].p[1]);

                    uint32_t cmin = (x != y ? 0 : r + 1);
                    for (uint32_t c = cmin; c < blockSize; ++c)
                    {
                        int32_t sx = origin[0] + c;
                        Vector3<double> P0 = HProject(segment[sx].p[0]);
                        Vector3<double> P1 = HProject(segment[sx].p[1]);

                        Result result0 = gpuResult[c + blockSize * r];
                        double distance0 = std::sqrt(result0.sqrDistance);

                        RobustQuery query1{};
                        auto result1 = query1.ComputeRobust(P0, P1, Q0, Q1);
                        double distance1 = result1.distance;

                        double error = std::fabs(distance0 - distance1);
                        if (error > maxError)
                        {
                            maxError = error;
                            xmax = sx;
                            ymax = sy;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // GLSL wants closest[] to be aligned on a dvec4 boundary, so
        // parameter[2] is padding.
        struct Result
        {
            double sqrDistance;
            double parameter[3];
        };

        output = std::make_shared<StructuredBuffer>(blockSize * blockSize, sizeof(Result));
        output->SetUsage(Resource::Usage::SHADER_OUTPUT);
        output->SetCopy(Resource::Copy::STAGING_TO_CPU);
        Result* gpuResult = output->Get<Result>();
        cshader->Set("outResult", output);

        for (uint32_t y = 0, i = 0; y < numBlocks; ++y)
        {
            std::cout << "block = " << y << std::endl;
            origin[1] = y * blockSize;
            for (uint32_t x = y; x < numBlocks; ++x, ++i)
            {
                origin[0] = x * blockSize;
                mEngine->Update(block);
                mEngine->Execute(cprogram, numGroups, numGroups, 1);
                mEngine->CopyGpuToCpu(output);

                for (uint32_t r = 0; r < blockSize; ++r)
                {
                    int32_t sy = origin[1] + r;
                    Vector3<double> Q0 = HProject(segment[sy].p[0]);
                    Vector3<double> Q1 = HProject(segment[sy].p[1]);

                    uint32_t cmin = (x != y ? 0 : r + 1);
                    for (uint32_t c = cmin; c < blockSize; ++c)
                    {
                        int32_t sx = origin[0] + c;
                        Vector3<double> P0 = HProject(segment[sx].p[0]);
                        Vector3<double> P1 = HProject(segment[sx].p[1]);

                        Result result0 = gpuResult[c + blockSize * r];
                        double distance0 = std::sqrt(result0.sqrDistance);

                        RobustQuery query1{};
                        auto result1 = query1.ComputeRobust(P0, P1, Q0, Q1);
                        double distance1 = result1.distance;

                        double error = std::fabs(distance0 - distance1);
                        if (error > maxError)
                        {
                            maxError = error;
                            xmax = sx;
                            ymax = sy;
                        }
                    }
                }
            }
        }
    }

    std::cout << "max error = " << maxError << std::endl;
    std::cout << "x, y = " << xmax << " " << ymax << std::endl;
    mOutput << "max error = " << maxError << std::endl;
    mOutput << "x, y = " << xmax << " " << ymax << std::endl;
}

void DistanceSegments3Console::GPUPerformanceTest(bool getClosest, bool testNonparallel)
{
    uint32_t const numInputs = 4096;
    uint32_t const blockSize = 1024;
    uint32_t const numBlocks = numInputs / blockSize;
    uint32_t const numThreads = 8;
    uint32_t const numGroups = blockSize / numThreads;
    uint32_t numQueriesPerCall = numGroups * numGroups * numThreads * numThreads;
    uint32_t numQueries;
    // The number of queries is:
    //   (sum_{n=1}^{numBlocks} n) * numGroups^2 * numThreads^2

    mProgramFactory->defines.Set("NUM_X_THREADS", numThreads);
    mProgramFactory->defines.Set("NUM_Y_THREADS", numThreads);
    mProgramFactory->defines.Set("BLOCK_SIZE", blockSize);
    mProgramFactory->defines.Set("REAL", "double");
#if defined(GTE_USE_OPENGL)
    mProgramFactory->defines.Set("VECREAL", "dvec4");
#endif
#if defined(GTE_USE_DIRECTX)
    mProgramFactory->defines.Set("VECREAL", "double4");
#endif
    mProgramFactory->defines.Set("GET_CLOSEST", (getClosest ? 1 : 0));
    auto cprogram = mProgramFactory->CreateFromFile(
        mEnvironment.GetPath(mEngine->GetShaderName("DistanceSeg3Seg3.cs")));
    auto const& cshader = cprogram->GetComputeShader();

    auto block = std::make_shared<ConstantBuffer>(2 * sizeof(uint32_t), true);
    cshader->Set("Block", block);
    auto* origin = block->Get<uint32_t>();

    auto input = std::make_shared<StructuredBuffer>(numInputs, sizeof(Segment<4, double>));
    input->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    cshader->Set("inSegment", input);
    Segment<4, double>* segment = input->Get<Segment<4, double>>();

    LoadInput(testNonparallel, numInputs, segment);

    Timer timer;
    std::shared_ptr<StructuredBuffer> output;
    if (getClosest)
    {
        // GLSL wants closest[] to be aligned on a dvec4 boundary, so
        // parameter[2] is padding.
        struct Result
        {
            double sqrDistance;
            double parameter[3];
            Vector3<double> closest[2];
        };

        output = std::make_shared<StructuredBuffer>(blockSize * blockSize, sizeof(Result));
        output->SetUsage(Resource::Usage::SHADER_OUTPUT);
        output->SetCopy(Resource::Copy::STAGING_TO_CPU);
        cshader->Set("outResult", output);

        numQueries = 0;
        for (uint32_t y = 0; y < numBlocks; ++y)
        {
            origin[1] = y * blockSize;
            for (uint32_t x = y; x < numBlocks; ++x)
            {
                origin[0] = x * blockSize;
                mEngine->Update(block);
                mEngine->Execute(cprogram, numGroups, numGroups, 1);
                mEngine->CopyGpuToCpu(output);
                numQueries += numQueriesPerCall;
            }
        }
    }
    else
    {
        // GLSL wants closest[] to be aligned on a dvec4 boundary, so
        // parameter[2] is padding.
        struct Result
        {
            double sqrDistance;
            double parameter[3];
        };

        output = std::make_shared<StructuredBuffer>(blockSize * blockSize, sizeof(Result));
        output->SetUsage(Resource::Usage::SHADER_OUTPUT);
        output->SetCopy(Resource::Copy::STAGING_TO_CPU);
        cshader->Set("outResult", output);

        numQueries = 0;
        for (uint32_t y = 0; y < numBlocks; ++y)
        {
            origin[1] = y * blockSize;
            for (uint32_t x = y; x < numBlocks; ++x)
            {
                origin[0] = x * blockSize;
                mEngine->Update(block);
                mEngine->Execute(cprogram, numGroups, numGroups, 1);
                mEngine->CopyGpuToCpu(output);
                numQueries += numQueriesPerCall;
            }
        }
    }
    double seconds = timer.GetSeconds();
    double secondsPerQuery = seconds / static_cast<double>(numQueries);
    std::cout
        << "seconds = " << seconds << ", "
        << "numQueries = " << numQueries << ", "
        << "seconds per query = " << secondsPerQuery << std::endl;
    mOutput
        << "seconds = " << seconds << ", "
        << "numQueries = " << numQueries << ", "
        << "seconds per query = " << secondsPerQuery << std::endl;
}
