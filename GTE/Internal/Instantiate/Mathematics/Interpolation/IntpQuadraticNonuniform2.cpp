#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Delaunay2Mesh.h>
#include <Mathematics/IntpQuadraticNonuniform2.h>

namespace gte
{
    using Numeric = BSNumber<UIntegerAP32>;
    using Rational = BSRational<UIntegerAP32>;
    using TriangleMeshF = Delaunay2Mesh<float, Numeric, Rational>;
    using TriangleMeshD = Delaunay2Mesh<double, Numeric, Rational>;

    template class IntpQuadraticNonuniform2<float, TriangleMeshF>;
    template class IntpQuadraticNonuniform2<double, TriangleMeshD>;
}
