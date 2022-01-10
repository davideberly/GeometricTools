#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/PlanarMesh.h>

namespace gte
{
    using Numeric = BSNumber<UIntegerAP32>;
    using Rational = BSRational<UIntegerAP32>;
    template class PlanarMesh<float, Numeric, Rational>;
    template class PlanarMesh<double, Numeric, Rational>;
}
