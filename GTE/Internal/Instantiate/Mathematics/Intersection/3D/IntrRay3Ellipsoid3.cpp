#include <Mathematics/IntrRay3Ellipsoid3.h>

namespace gte
{
    template class TIQuery<float, Ray3<float>, Ellipsoid3<float>>;
    template class FIQuery<float, Ray3<float>, Ellipsoid3<float>>;

    template class TIQuery<double, Ray3<double>, Ellipsoid3<double>>;
    template class FIQuery<double, Ray3<double>, Ellipsoid3<double>>;
}
