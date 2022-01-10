#include <Mathematics/IntrPlane3Sphere3.h>

namespace gte
{
    template class TIQuery<float, Plane3<float>, Sphere3<float>>;
    template class FIQuery<float, Plane3<float>, Sphere3<float>>;

    template class TIQuery<double, Plane3<double>, Sphere3<double>>;
    template class FIQuery<double, Plane3<double>, Sphere3<double>>;
}
