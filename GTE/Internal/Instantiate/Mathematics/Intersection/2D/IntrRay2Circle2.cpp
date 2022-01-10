#include <Mathematics/IntrRay2Circle2.h>

namespace gte
{
    template class TIQuery<float, Ray2<float>, Circle2<float>>;
    template class FIQuery<float, Ray2<float>, Circle2<float>>;

    template class TIQuery<double, Ray2<double>, Circle2<double>>;
    template class FIQuery<double, Ray2<double>, Circle2<double>>;
}
