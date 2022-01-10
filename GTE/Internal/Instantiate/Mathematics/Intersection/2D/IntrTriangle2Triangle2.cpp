#include <Mathematics/IntrTriangle2Triangle2.h>

namespace gte
{
    template class TIQuery<float, Triangle2<float>, Triangle2<float>>;
    template class FIQuery<float, Triangle2<float>, Triangle2<float>>;

    template class TIQuery<double, Triangle2<double>, Triangle2<double>>;
    template class FIQuery<double, Triangle2<double>, Triangle2<double>>;
}
