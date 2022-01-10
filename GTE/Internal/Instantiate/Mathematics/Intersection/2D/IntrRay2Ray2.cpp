#include <Mathematics/IntrRay2Ray2.h>

namespace gte
{
    template class TIQuery<float, Ray2<float>, Ray2<float>>;
    template class FIQuery<float, Ray2<float>, Ray2<float>>;

    template class TIQuery<double, Ray2<double>, Ray2<double>>;
    template class FIQuery<double, Ray2<double>, Ray2<double>>;
}
