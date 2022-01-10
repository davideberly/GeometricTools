#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/IntpVectorField2.h>

namespace gte
{
    using Numeric = BSNumber<UIntegerAP32>;
    using Rational = BSRational<UIntegerAP32>;
    template class IntpVectorField2<float, Numeric, Rational>;
    template class IntpVectorField2<double, Numeric, Rational>;
}
