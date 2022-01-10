#define GTE_COLLECT_UINTEGERAP32_STATISTICS
#include <Mathematics/UIntegerAP32.h>

namespace gte
{
    std::atomic<size_t> gsUIntegerAP32MaxSize;
    template class UIntegerALU32<UIntegerAP32>;
}
