#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/TriangulateCDT.h>

namespace gte
{
    template class TriangulateCDT<float, float>;
    template class TriangulateCDT<double, double>;
    template class TriangulateCDT<float, BSNumber<UIntegerAP32>>;
    template class TriangulateCDT<double, BSNumber<UIntegerFP32<526>>>;

    template class TriangulateCDT<float>;
    template class TriangulateCDT<double>;
}
