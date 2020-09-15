#include "MinimumVolumeBox.h"
#include <Mathematics/MinimumVolumeBox3.h>
using namespace gte;

MVB3::MVB3()
{
}

void MVB3::ComputeMinimumVolumeBoxFromPoints(unsigned int numThreads,
    int numPoints, double const* points, unsigned int lgMaxSample,
    double center[3], double axis[9], double extent[3], double volume[1])
{
    if (numPoints > 0 && points)
    {
        MinimumVolumeBox3<double, true> mvb(numThreads);
        auto const* vpoints = reinterpret_cast<Vector3<double> const*>(points);
        OrientedBox3<double> box;
        mvb(numPoints, vpoints, lgMaxSample, box, volume[0]);
        for (uint32_t i = 0; i < 3; ++i)
        {
            center[i] = box.center[i];
            extent[i] = box.extent[i];
            for (uint32_t j = 0; j < 3; ++j)
            {
                axis[3 * i + j] = box.axis[i][j];
            }
        }
    }
    else
    {
        for (uint32_t i = 0; i < 3; ++i)
        {
            center[i] = 0.0;
            extent[i] = 0.0;
            for (uint32_t j = 0; j < 3; ++j)
            {
                axis[3 * i + j] = 0.0;
            }
        }
        volume[0] = 0.0;
    }
}

void MVB3::ComputeMinimumVolumeBoxFromPolyhedron(unsigned int numThreads,
    int numPoints, double const* points, int numIndices, int const* indices,
    unsigned int lgMaxSample,
    double center[3], double axis[9], double extent[3], double volume[1])
{
    if (numPoints > 0 && points && numIndices > 0 && indices)
    {
        MinimumVolumeBox3<double, true> mvb(numThreads);
        auto const* vpoints = reinterpret_cast<Vector3<double> const*>(points);
        OrientedBox3<double> box;
        mvb(numPoints, vpoints, numIndices, indices, lgMaxSample, box, volume[0]);
        for (uint32_t i = 0; i < 3; ++i)
        {
            center[i] = box.center[i];
            extent[i] = box.extent[i];
            for (uint32_t j = 0; j < 3; ++j)
            {
                axis[3 * i + j] = box.axis[i][j];
            }
        }
    }
    else
    {
        for (uint32_t i = 0; i < 3; ++i)
        {
            center[i] = 0.0;
            extent[i] = 0.0;
            for (uint32_t j = 0; j < 3; ++j)
            {
                axis[3 * i + j] = 0.0;
            }
        }
        volume[0] = 0.0;
    }
}
