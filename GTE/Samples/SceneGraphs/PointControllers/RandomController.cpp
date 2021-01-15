// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "RandomController.h"
#include <Graphics/Visual.h>
using namespace gte;

RandomController::RandomController(BufferUpdater const& postUpdate)
    :
    PointController(postUpdate),
    mURD(-0.01f, 0.01f)
{
}

void RandomController::UpdatePointMotion(float)
{
    auto visual = reinterpret_cast<Visual*>(mObject);
    auto vbuffer = visual->GetVertexBuffer();
    VertexFormat vformat = vbuffer->GetFormat();
    unsigned int numVertices = vbuffer->GetNumElements();
    char* vertices = vbuffer->GetData();
    size_t vertexSize = static_cast<size_t>(vformat.GetVertexSize());

    for (unsigned int i = 0; i < numVertices; ++i)
    {
        auto& position = *reinterpret_cast<Vector3<float>*>(vertices);
        for (int j = 0; j < 3; ++j)
        {
            position[j] += mURD(mDRE);
            if (position[j] > 1.0f)
            {
                position[j] = 1.0f;
            }
            else if (position[j] < -1.0f)
            {
                position[j] = -1.0f;
            }
        }
        vertices += vertexSize;
    }

    mPostUpdate(vbuffer);
}
