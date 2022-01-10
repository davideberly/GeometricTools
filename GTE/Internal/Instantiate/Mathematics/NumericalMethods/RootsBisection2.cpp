#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/RootsBisection2.h>

namespace gte
{
    template class RootsBisection2<float>;
    template class RootsBisection2<double>;
    template class RootsBisection2<BSRational<UIntegerAP32>>;
    template class RootsBisection2<BSRational<UIntegerFP32<32>>>;
}
