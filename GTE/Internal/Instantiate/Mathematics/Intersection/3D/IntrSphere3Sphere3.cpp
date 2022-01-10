#include <Mathematics/IntrSphere3Sphere3.h>

namespace gte
{
    template class TIQuery<float, Sphere3<float>, Sphere3<float>>;
    template class FIQuery<float, Sphere3<float>, Sphere3<float>>;

    template class TIQuery<double, Sphere3<double>, Sphere3<double>>;
    template class FIQuery<double, Sphere3<double>, Sphere3<double>>;
}
