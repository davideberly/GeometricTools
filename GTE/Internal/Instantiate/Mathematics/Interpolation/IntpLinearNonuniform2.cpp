#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/IntpLinearNonuniform2.h>
#include <Mathematics/Delaunay2Mesh.h>

namespace gte
{
    using Numeric = BSNumber<UIntegerAP32>;
    using Rational = BSRational<UIntegerAP32>;
    using TriangleMeshF = Delaunay2Mesh<float, Numeric, Rational>;
    using TriangleMeshD = Delaunay2Mesh<double, Numeric, Rational>;

    template class IntpLinearNonuniform2<float, TriangleMeshF>;
    template class IntpLinearNonuniform2<double, TriangleMeshD>;
}
