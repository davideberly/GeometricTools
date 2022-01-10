#include <Mathematics/DistPlane3OrientedBox3.h>

namespace gte
{
    template class DCPQuery<float, Plane3<float>, OrientedBox3<float>>;
    template class DCPQuery<double, Plane3<double>, OrientedBox3<double>>;
}
