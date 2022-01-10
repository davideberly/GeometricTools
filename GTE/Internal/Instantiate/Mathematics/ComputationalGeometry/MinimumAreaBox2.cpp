#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/MinimumAreaBox2.h>

namespace gte
{
    template class MinimumAreaBox2<float, BSRational<UIntegerAP32>>;
    template class MinimumAreaBox2<float, float>;

    template class MinimumAreaBox2<double, BSRational<UIntegerAP32>>;
    template class MinimumAreaBox2<double, double>;
}
