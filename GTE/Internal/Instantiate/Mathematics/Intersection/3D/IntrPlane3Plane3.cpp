#include <Mathematics/IntrPlane3Plane3.h>

namespace gte
{
    template class TIQuery<float, Plane3<float>, Plane3<float>>;
    template class FIQuery<float, Plane3<float>, Plane3<float>>;

    template class TIQuery<double, Plane3<double>, Plane3<double>>;
    template class FIQuery<double, Plane3<double>, Plane3<double>>;
}
