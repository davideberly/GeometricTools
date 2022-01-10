#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Delaunay3Mesh.h>

namespace gte
{
    typedef BSNumber<UIntegerAP32> Numeric;
    typedef BSRational<UIntegerAP32> Rational;
    template class Delaunay3Mesh<float, Numeric, Rational>;
    template class Delaunay3Mesh<double, Numeric, Rational>;
}
