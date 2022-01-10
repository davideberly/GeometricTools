#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/QFNumber.h>

namespace gte
{
    template class QFNumber<float, 1>;
    template class QFNumber<double, 1>;
    template class QFNumber<BSRational<UIntegerAP32>, 1>;
    template class QFNumber<BSRational<UIntegerFP32<4>>, 1>;

    template class QFNumber<float, 2>;
    template class QFNumber<double, 2>;
    template class QFNumber<BSRational<UIntegerAP32>, 2>;
    template class QFNumber<BSRational<UIntegerFP32<4>>, 2>;
}
