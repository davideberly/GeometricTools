#include <Mathematics/IntrPlane3Triangle3.h>

namespace gte
{
    template class TIQuery<float, Plane3<float>, Triangle3<float>>;
    template class FIQuery<float, Plane3<float>, Triangle3<float>>;

    template class TIQuery<double, Plane3<double>, Triangle3<double>>;
    template class FIQuery<double, Plane3<double>, Triangle3<double>>;
}
