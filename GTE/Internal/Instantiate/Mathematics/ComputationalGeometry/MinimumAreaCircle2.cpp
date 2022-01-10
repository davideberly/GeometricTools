#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/MinimumAreaCircle2.h>

namespace gte
{
    template class MinimumAreaCircle2<float, BSRational<UIntegerAP32>>;
    template class MinimumAreaCircle2<float, float>;

    template class MinimumAreaCircle2<double, BSRational<UIntegerAP32>>;
    template class MinimumAreaCircle2<double, double>;
}
