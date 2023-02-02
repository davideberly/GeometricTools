// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/IntrSphere3Triangle3.h>
using namespace gte;

// Comment out this line to use the arbitrary-precision query for the
// intersection of a moving sphere and triangle.
#define USE_FLOATING_POINT_QUERY

class MovingSphereTriangleWindow3 : public Window3
{
public:
    MovingSphereTriangleWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();
    void CreateTriangleFaces();
    void CreateHalfCylinders();
    void CreateSphereWedges();
    void CreateSpheres();
    void CreateMotionCylinder();
    void UpdateSphereVelocity();
    void UpdateSphereCenter();

    void CreateHalfCylinder(int32_t i, Vector3<float> const& P0, Vector3<float> const& P1,
        Vector3<float> const& normal, float radius);

    void CreateSphere(int32_t i, Vector3<float> const& C, float radius);

    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<RasterizerState> mNoCullState;
    float mAlpha;

    std::shared_ptr<Node> mSSVNode;

    // Sphere wedges for the triangle vertices.
    std::array<std::shared_ptr<Visual>, 3> mVertexVisual;

    // Half cylinders for the triangle edges.
    std::array<std::shared_ptr<Visual>, 3> mEdgeVisual;

    // Triangle faces.
    std::array<std::shared_ptr<Visual>, 2> mFaceVisual;

    // The visual representation of mSphere.
    std::shared_ptr<Visual> mSphereVisual;
    std::shared_ptr<Visual> mSphereContactVisual;

    // The visual representation of mTriangle.
    std::shared_ptr<Visual> mTriangleVisual;

    // The visual representation of the moving path of the sphere.
    std::shared_ptr<Visual> mVelocityVisual;

    // The contact point representation.
    std::shared_ptr<Visual> mPointContactVisual;

    Sphere3<float> mSphere;
    Triangle3<float> mTriangle;
    Vector3<float> mTriangleNormal;
    Vector3<float> mSphereVelocity;
    Vector3<float> mTriangleVelocity;

#if defined(USE_FLOATING_POINT_QUERY)
    FIQuery<float, Sphere3<float>, Triangle3<float>> mQuery;
#else
    typedef BSRational<UIntegerAP32> Rational;
    FIQuery<Rational, Sphere3<Rational>, Triangle3<Rational>> mQuery;
#endif

    int32_t mNumSamples0, mNumSamples1, mSample0, mSample1;
    float mDX, mDY, mDZ;
    std::string mMessage;
    bool mDrawSphereVisual;
};
