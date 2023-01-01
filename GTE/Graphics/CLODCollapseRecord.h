// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.02.24

#pragma once

#include <cstdint>
#include <vector>

namespace gte
{
    class CLODCollapseRecord
    {
    public:
        CLODCollapseRecord(int32_t inVKeep = -1, int32_t inVThrow = -1,
            int32_t inNumVertices = 0, int32_t inNumTriangles = 0)
            :
            vKeep(inVKeep),
            vThrow(inVThrow),
            numVertices(inNumVertices),
            numTriangles(inNumTriangles),
            indices{}
        {
        }

        // Edge <VKeep,VThrow> collapses so that VThrow is replaced by VKeep.
        int32_t vKeep, vThrow;

        // The number of vertices after the edge collapse.
        int32_t numVertices;

        // The number of triangles after the edge collapse.
        int32_t numTriangles;

        // The array of indices in [0..numTriangles-1] that contain vThrow.
        std::vector<int32_t> indices;
    };
}
