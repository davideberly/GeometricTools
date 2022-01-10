#include <Mathematics/IntrRay2OrientedBox2.h>

namespace gte
{
    template class TIQuery<float, Ray2<float>, OrientedBox2<float>>;
    template class FIQuery<float, Ray2<float>, OrientedBox2<float>>;

    template class TIQuery<double, Ray2<double>, OrientedBox2<double>>;
    template class FIQuery<double, Ray2<double>, OrientedBox2<double>>;
}
