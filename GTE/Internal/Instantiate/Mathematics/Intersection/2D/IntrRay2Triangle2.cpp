#include <Mathematics/IntrRay2Triangle2.h>

namespace gte
{
    template class TIQuery<float, Ray2<float>, Triangle2<float>>;
    template class FIQuery<float, Ray2<float>, Triangle2<float>>;

    template class TIQuery<double, Ray2<double>, Triangle2<double>>;
    template class FIQuery<double, Ray2<double>, Triangle2<double>>;
}
