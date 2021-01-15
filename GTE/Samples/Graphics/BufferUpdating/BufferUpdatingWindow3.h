// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
using namespace gte;

class BufferUpdatingWindow3 : public Window3
{
public:
    BufferUpdatingWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // The surface is a height field of NUM_SAMPLES-by-NUM_SAMPLES vertices.
    enum { NUM_SAMPLES = 1024 };
    std::shared_ptr<Visual> mSurface;
    std::shared_ptr<RasterizerState> mWireState;
};
