// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
#include "TriangleIntersection.h"
using namespace gte;

//#define USE_CPU_FIND_INTERSECTIONS

class AllPairsTrianglesWindow3 : public Window3
{
public:
    AllPairsTrianglesWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();

    bool CreateCylinder(unsigned int numAxisSamples,
        unsigned int numRadialSamples, float radius, float height);

    bool CreateTorus(unsigned int numCircleSamples,
        unsigned int numRadialSamples, float outerRadius, float innerRadius);

#if !defined(USE_CPU_FIND_INTERSECTIONS)
    bool CreateShaders();
#endif

    void UpdateTransforms();
    void FindIntersections();

    Environment mEnvironment;
    Vector4<float> mTextColor;
    std::shared_ptr<RasterizerState> mWireState;

    struct Vertex
    {
        Vector3<float> position;
        float colorIndex;
    };

    unsigned int mNumCylinderTriangles, mNumTorusTriangles;
    std::shared_ptr<Visual> mCylinder, mTorus;
    std::shared_ptr<VisualEffect> mCylinderEffect, mTorusEffect;
    std::shared_ptr<ConstantBuffer> mCylinderPVWMatrix, mTorusPVWMatrix;

#if !defined(USE_CPU_FIND_INTERSECTIONS)
    unsigned int mNumXGroups, mNumYGroups;

    std::shared_ptr<StructuredBuffer> mColor0Buffer, mColor1Buffer;
    std::shared_ptr<ComputeProgram> mInitializeColor;

    struct TIParameters
    {
        Matrix4x4<float> wMatrix0, wMatrix1;
        unsigned int numTriangles0, numTriangles1;
    };
    std::shared_ptr<ConstantBuffer> mTIParameters;
    std::shared_ptr<StructuredBuffer> mVertices0, mVertices1;
    std::shared_ptr<ComputeProgram> mTriangleIntersection;

    std::shared_ptr<VisualEffect> mCylinderIDEffect, mTorusIDEffect;
    std::shared_ptr<Visual> mCylinderID, mTorusID;
#endif
};
