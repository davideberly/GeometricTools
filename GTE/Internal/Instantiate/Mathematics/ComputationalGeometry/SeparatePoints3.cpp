#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/SeparatePoints3.h>

namespace gte
{
    template class SeparatePoints3<float, BSNumber<UIntegerAP32>>;
    template class SeparatePoints3<float, float>;

    template class SeparatePoints3<double, BSNumber<UIntegerAP32>>;
    template class SeparatePoints3<double, double>;
}
