// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

// Expose this for the BSP-based query.  Comment it out for the
// projection-based query.
//#define USE_BSP_QUERY

// Uncomment this for timing information.
//#define MEASURE_TIMING_OF_QUERY

#include <Applications/Window3.h>
#include <Mathematics/Polyhedron3.h>
#if defined(USE_BSP_QUERY)
#include <Mathematics/ExtremalQuery3BSP.h>
#else
#include <Mathematics/ExtremalQuery3PRJ.h>
#endif
using namespace gte;

class ExtremalQueryWindow3 : public Window3
{
public:
    ExtremalQueryWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;
    virtual bool OnMouseMotion(MouseButton button, int32_t x, int32_t y, uint32_t modifiers) override;

private:
    void CreateScene();
    void CreateConvexPolyhedron(int32_t numVertices);
    void CreateVisualConvexPolyhedron();
    void UpdateExtremePoints();

    std::unique_ptr<Polyhedron3<float>> mConvexPolyhedron;
    std::unique_ptr<ExtremalQuery3<float>> mExtremalQuery;
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mConvexMesh, mMaxSphere, mMinSphere;
    std::shared_ptr<RasterizerState> mWireState;
};
