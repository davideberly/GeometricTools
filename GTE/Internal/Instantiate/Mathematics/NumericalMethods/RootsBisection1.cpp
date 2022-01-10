#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/RootsBisection1.h>

namespace gte
{
    template class RootsBisection1<float>;
    template class RootsBisection1<double>;
    template class RootsBisection1<BSRational<UIntegerAP32>>;
    template class RootsBisection1<BSRational<UIntegerFP32<32>>>;
}
