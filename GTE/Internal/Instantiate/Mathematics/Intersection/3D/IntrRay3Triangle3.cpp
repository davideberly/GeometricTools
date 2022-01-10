#include <Mathematics/IntrRay3Triangle3.h>

namespace gte
{
    template class TIQuery<float, Ray3<float>, Triangle3<float>>;
    template class FIQuery<float, Ray3<float>, Triangle3<float>>;

    template class TIQuery<double, Ray3<double>, Triangle3<double>>;
    template class FIQuery<double, Ray3<double>, Triangle3<double>>;
}
