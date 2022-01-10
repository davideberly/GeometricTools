#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/MinimumVolumeSphere3.h>

namespace gte
{
    template class MinimumVolumeSphere3<float, BSRational<UIntegerAP32>>;
    template class MinimumVolumeSphere3<float, float>;
}
