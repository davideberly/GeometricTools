#include <Mathematics/IntrRay3Capsule3.h>

namespace gte
{
    template class TIQuery<float, Ray3<float>, Capsule3<float>>;
    template class FIQuery<float, Ray3<float>, Capsule3<float>>;

    template class TIQuery<double, Ray3<double>, Capsule3<double>>;
    template class FIQuery<double, Ray3<double>, Capsule3<double>>;
}
