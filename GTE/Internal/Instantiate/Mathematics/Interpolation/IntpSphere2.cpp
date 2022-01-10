#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/IntpSphere2.h>

namespace gte
{
    using Numeric = BSNumber<UIntegerAP32>;
    using Rational = BSRational<UIntegerAP32>;
    template class IntpSphere2<float, Numeric, Rational>;
    template class IntpSphere2<double, Numeric, Rational>;
}
