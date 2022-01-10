#include <Mathematics/IntrPlane3Circle3.h>

namespace gte
{
    template class TIQuery<float, Plane3<float>, Circle3<float>>;
    template class FIQuery<float, Plane3<float>, Circle3<float>>;

    template class TIQuery<double, Plane3<double>, Circle3<double>>;
    template class FIQuery<double, Plane3<double>, Circle3<double>>;
}
