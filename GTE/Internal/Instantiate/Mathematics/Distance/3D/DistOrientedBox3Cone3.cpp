#include <Mathematics/DistOrientedBox3Cone3.h>

namespace gte
{
    template class DCPQuery<float, OrientedBox3<float>, Cone3<float>>;
    template class DCPQuery<double, OrientedBox3<double>, Cone3<double>>;
}
