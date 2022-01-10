#include <Mathematics/IntrPlane3Cylinder3.h>

namespace gte
{
    template class TIQuery<float, Plane3<float>, Cylinder3<float>>;
    template class FIQuery<float, Plane3<float>, Cylinder3<float>>;

    template class TIQuery<double, Plane3<double>, Cylinder3<double>>;
    template class FIQuery<double, Plane3<double>, Cylinder3<double>>;
}
