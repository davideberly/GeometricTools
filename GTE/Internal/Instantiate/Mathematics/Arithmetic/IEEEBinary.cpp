#include <Mathematics/IEEEBinary.h>

namespace gte
{
    template class IEEEBinary<float, uint32_t, 32, 24>;
    template class IEEEBinary<double, uint64_t, 64, 53>;
}
