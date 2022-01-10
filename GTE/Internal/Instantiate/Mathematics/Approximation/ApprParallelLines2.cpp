#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/ApprParallelLines2.h>

namespace gte
{
    template class ApprParallelLines2<float>;
    template class ApprParallelLines2<double>;

    using Rational = BSRational<UIntegerAP32>;
    template class ApprParallelLines2<Rational>;
}
