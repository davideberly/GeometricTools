#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Delaunay2Mesh.h>

namespace gte
{
    typedef BSNumber<UIntegerAP32> Numeric;
    typedef BSRational<UIntegerAP32> Rational;
    template class Delaunay2Mesh<float, Numeric, Rational>;
    template class Delaunay2Mesh<double, Numeric, Rational>;
}
