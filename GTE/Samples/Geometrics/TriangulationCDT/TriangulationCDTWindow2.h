// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/PlanarMesh.h>
#include <Mathematics/TriangulateCDT.h>
using namespace gte;

class TriangulationCDTWindow2 : public Window2
{
public:
    TriangulationCDTWindow2(Parameters& parameters);

    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void DrawTriangulation();

    void UnindexedSimplePolygon();  // key = '0'
    void IndexedSimplePolygon();    // key = '1'
    void OneNestedPolygon();        // key = '2'
    void TwoNestedPolygons();       // key = '3'
    void TreeOfNestedPolygons();    // key = '4'

    typedef BSNumber<UIntegerAP32> Rational;
    typedef TriangulateCDT<float, Rational> Triangulator;
    typedef PlanarMesh<float, Rational, Rational> PlanarMesher;
    std::vector<Vector2<float>> mPoints;
    std::unique_ptr<Triangulator> mTriangulator;
    std::unique_ptr<PlanarMesher> mPMesher;
};
