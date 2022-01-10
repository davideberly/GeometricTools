#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/IntpLinearNonuniform3.h>
#include <Mathematics/Delaunay3Mesh.h>

namespace gte
{
    using Numeric = BSNumber<UIntegerAP32>;
    using Rational = BSRational<UIntegerAP32>;
    using TriangleMeshF = Delaunay3Mesh<float, Numeric, Rational>;
    using TriangleMeshD = Delaunay3Mesh<double, Numeric, Rational>;

    template class IntpLinearNonuniform3<float, TriangleMeshF>;
    template class IntpLinearNonuniform3<double, TriangleMeshD>;
}
