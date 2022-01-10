#include <Mathematics/IntrRay3OrientedBox3.h>

namespace gte
{
    template class TIQuery<float, Ray3<float>, OrientedBox3<float>>;
    template class FIQuery<float, Ray3<float>, OrientedBox3<float>>;

    template class TIQuery<double, Ray3<double>, OrientedBox3<double>>;
    template class FIQuery<double, Ray3<double>, OrientedBox3<double>>;
}
