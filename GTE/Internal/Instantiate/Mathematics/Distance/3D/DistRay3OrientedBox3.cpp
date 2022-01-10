#include <Mathematics/DistRay3OrientedBox3.h>

namespace gte
{
    template class DCPQuery<float, Ray3<float>, OrientedBox3<float>>;
    template class DCPQuery<double, Ray3<double>, OrientedBox3<double>>;
}
