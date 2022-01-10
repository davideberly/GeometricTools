#include <Mathematics/ArbitraryPrecision.h>
#include "BooleanPolygonWindow2.h"
#include "BooleanPolygon2.h"
#include "IntrSegment2AllPairs.h"
#include "SegmentMeshProcessor.h"

namespace gte
{
    template class SegmentMeshProcessor<float>;

    //using Rational = BSRational<UIntegerAP32>;
    //template class IntrSegment2AllPairs<float, Rational>;
    //template class IntrSegment2AllPairs<Rational, Rational>;
    //template class Polygon<float>;
}

BooleanPolygonWindow2::BooleanPolygonWindow2(Parameters& parameters)
    :
    Window2(parameters)
{
    double const size = 512.0;
    double d1d8 = 0.125 * size;
    double d2d8 = 0.250 * size;
    double d3d8 = 0.375 * size;
    double d5d8 = 0.625 * size;
    double d6d8 = 0.750 * size;
    double d7d8 = 0.875 * size;
    std::vector<Vector2<double>> polygon0 =
    {
        Vector2<double>{ d1d8, d1d8 },
        Vector2<double>{ d3d8, d1d8 },
        Vector2<double>{ d3d8, d3d8 },
        Vector2<double>{ d2d8, d3d8 },
        Vector2<double>{ d2d8, d6d8 },
        Vector2<double>{ d5d8, d6d8 },
        Vector2<double>{ d5d8, d5d8 },
        Vector2<double>{ d7d8, d5d8 },
        Vector2<double>{ d7d8, d7d8 },
        Vector2<double>{ d1d8, d7d8 }
    };

    double primitiveAngle = GTE_C_TWO_PI / 5.0;
    double radius = 0.35 * size;
    double cx = 0.5 * size;
    double cy = 0.5 * size;
    std::vector<Vector2<double>> polygon1(5);
    for (size_t i = 0; i < 5; ++i)
    {
        double angle = i * primitiveAngle;
        polygon1[i][0] = cx + radius * std::cos(angle);
        polygon1[i][1] = cy + radius * std::sin(angle);
    }

    BooleanPolygon2<double> bp2;
    std::vector<std::vector<Vector2<double>>> output;
    bp2.Intersection(polygon0, polygon1, output);

    mDoFlip = true;
    OnDisplay();
}

void BooleanPolygonWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}
