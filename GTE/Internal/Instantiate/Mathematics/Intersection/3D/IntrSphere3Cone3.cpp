#include <Mathematics/IntrSphere3Cone3.h>

namespace gte
{
    template class TIQuery<float, Sphere3<float>, Cone3<float>>;
    template class FIQuery<float, Sphere3<float>, Cone3<float>>;

    template class TIQuery<double, Sphere3<double>, Cone3<double>>;
    template class FIQuery<double, Sphere3<double>, Cone3<double>>;
}
