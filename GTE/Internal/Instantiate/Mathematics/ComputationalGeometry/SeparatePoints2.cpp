#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/SeparatePoints2.h>

namespace gte
{
    template class SeparatePoints2<float, BSNumber<UIntegerAP32>>;
    template class SeparatePoints2<float, float>;

    template class SeparatePoints2<double, BSNumber<UIntegerAP32>>;
    template class SeparatePoints2<double, double>;
}
