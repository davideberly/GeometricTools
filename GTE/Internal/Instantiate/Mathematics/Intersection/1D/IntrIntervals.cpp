#include <Mathematics/IntrIntervals.h>

namespace gte
{
    template class TIQuery<float, std::array<float, 2>, std::array<float, 2>>;
    template class FIQuery<float, std::array<float, 2>, std::array<float, 2>>;

    template class TIQuery<double, std::array<double, 2>, std::array<double, 2>>;
    template class FIQuery<double, std::array<double, 2>, std::array<double, 2>>;
}
