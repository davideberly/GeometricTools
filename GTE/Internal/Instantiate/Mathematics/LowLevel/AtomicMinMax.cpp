#include <Mathematics/AtomicMinMax.h>

namespace gte
{
    template float gte::AtomicMax(std::atomic<float>&, float const&);
    template float gte::AtomicMin(std::atomic<float>&, float const&);
}
