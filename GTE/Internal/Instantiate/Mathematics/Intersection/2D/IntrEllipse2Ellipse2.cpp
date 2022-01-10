#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/IntrEllipse2Ellipse2.h>

namespace gte
{
    using Rational = BSRational<UIntegerAP32>;

    template class TIQuery<float, Ellipse2<float>, Ellipse2<float>>;
    template class TIQuery<double, Ellipse2<double>, Ellipse2<double>>;
    template class TIQuery<Rational, Ellipse2<Rational>, Ellipse2<Rational>>;

    template class FIQuery<float, Ellipse2<float>, Ellipse2<float>>;
    template class FIQuery<double, Ellipse2<double>, Ellipse2<double>>;
    template class FIQuery<Rational, Ellipse2<Rational>, Ellipse2<Rational>>;
}
