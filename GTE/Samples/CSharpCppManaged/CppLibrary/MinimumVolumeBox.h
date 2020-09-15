#pragma once

namespace gte
{
    class MVB3
    {
    public:
        MVB3();

        void ComputeMinimumVolumeBoxFromPoints(unsigned int numThreads,
            int numPoints, double const* points, unsigned int lgMaxSample,
            double center[3], double axis[9], double extent[3],
            double volume[1]);

        void ComputeMinimumVolumeBoxFromPolyhedron(unsigned int numThreads,
            int numPoints, double const* points, int numIndices,
            int const* indices, unsigned int lgMaxSample,
            double center[3], double axis[9], double extent[3],
            double volume[1]);
    };
}
