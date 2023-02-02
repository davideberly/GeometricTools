// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/BSPPolygon2.h>
using namespace gte;

class PolygonBooleanOperationsWindow2 : public Window2
{
public:
    PolygonBooleanOperationsWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    std::unique_ptr<BSPPolygon2<double>> ConstructInvertedEll();
    std::unique_ptr<BSPPolygon2<double>> ConstructPentagon();
    std::unique_ptr<BSPPolygon2<double>> ConstructSquare();
    std::unique_ptr<BSPPolygon2<double>> ConstructSShape();
    std::unique_ptr<BSPPolygon2<double>> ConstructPolyWithHoles();

    void DoBoolean();
    void DrawPolySolid(BSPPolygon2<double>& polygon, uint32_t color);

    double mEpsilon;
    BSPPolygon2<double> mIntersection, mUnion, mDiff01, mDiff10, mXor;
    std::unique_ptr<BSPPolygon2<double>> mPoly0;
    std::unique_ptr<BSPPolygon2<double>> mPoly1;
    BSPPolygon2<double>* mActive;
    int32_t mChoice;
    double mSize;
};
