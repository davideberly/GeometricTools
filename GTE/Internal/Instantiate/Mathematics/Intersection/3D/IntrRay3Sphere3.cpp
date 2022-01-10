#include <Mathematics/IntrRay3Sphere3.h>

namespace gte
{
    template class TIQuery<float, Ray3<float>, Sphere3<float>>;
    template class FIQuery<float, Ray3<float>, Sphere3<float>>;

    template class TIQuery<double, Ray3<double>, Sphere3<double>>;
    template class FIQuery<double, Ray3<double>, Sphere3<double>>;
}
