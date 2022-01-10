#include <Mathematics/IntrOrientedBox3Sphere3.h>

namespace gte
{
    template class TIQuery<float, OrientedBox3<float>, Sphere3<float>>;
    template class FIQuery<float, OrientedBox3<float>, Sphere3<float>>;

    template class TIQuery<double, OrientedBox3<double>, Sphere3<double>>;
    template class FIQuery<double, OrientedBox3<double>, Sphere3<double>>;
}
