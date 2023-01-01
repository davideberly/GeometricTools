// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/ConvexHull2.h>
using namespace gte;

class ConvexHull2DWindow2 : public Window2
{
public:
    ConvexHull2DWindow2(Parameters& parameters);

    virtual void OnDisplay() override;

private:
    std::vector<Vector2<float>> mVertices;
    std::vector<int32_t> mHull;
    ConvexHull2<float> mConvexHull;
};
