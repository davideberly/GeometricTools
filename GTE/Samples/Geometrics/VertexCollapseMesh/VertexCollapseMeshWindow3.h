// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/VertexCollapseMesh.h>
using namespace gte;

class VertexCollapseMeshWindow3 : public Window3
{
public:
    VertexCollapseMeshWindow3(Parameters& parameters);

    virtual void OnIdle();
    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    Vector4<float> mTextColor;
    std::vector<Vector3<float>> mPositions;
    std::vector<std::array<int, 3>> mTriangles;
    std::shared_ptr<VertexCollapseMesh<float>> mVCMesh;
    std::shared_ptr<Visual> mSurface;
    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
};
