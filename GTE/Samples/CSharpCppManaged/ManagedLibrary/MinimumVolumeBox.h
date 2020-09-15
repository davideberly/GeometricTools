#pragma once
#include "ManagedObject.h"
#include "../CppLibrary/CppLibrary.h"
using namespace System;

namespace CLI
{
    public ref class MVB3 : public ManagedObject<gte::MVB3>
    {
    public:
        MVB3();

        void ComputeMinimumVolumeBoxFromPoints(unsigned int numThreads,
            int numPoints, array<double>^ points, unsigned int lgMaxSample,
            array<double>^ center, array<double>^ axis, array<double>^ extent,
            array<double>^ volume);

        void ComputeMinimumVolumeBoxFromPolyhedron(unsigned int numThreads,
            int numPoints, array<double>^ points, int numIndices,
            array<int>^ indices, unsigned int lgMaxSample,
            array<double>^ center, array<double>^ axis, array<double>^ extent,
            array<double>^ volume);
    };
}
