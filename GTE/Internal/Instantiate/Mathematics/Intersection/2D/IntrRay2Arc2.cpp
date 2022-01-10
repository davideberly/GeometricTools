#include <Mathematics/IntrRay2Arc2.h>

namespace gte
{
    template class TIQuery<float, Ray2<float>, Arc2<float>>;
    template class FIQuery<float, Ray2<float>, Arc2<float>>;

    template class TIQuery<double, Ray2<double>, Arc2<double>>;
    template class FIQuery<double, Ray2<double>, Arc2<double>>;
}
