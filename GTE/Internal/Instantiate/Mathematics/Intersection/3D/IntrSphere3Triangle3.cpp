#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/IntrSphere3Triangle3.h>

namespace gte
{
    using Rational = BSRational<UIntegerAP32>;

    template class FIQuery<float, Sphere3<float>, Triangle3<float>>;
    template class FIQuery<double, Sphere3<double>, Triangle3<double>>;
    template class FIQuery<Rational, Sphere3<Rational>, Triangle3<Rational>>;
}
