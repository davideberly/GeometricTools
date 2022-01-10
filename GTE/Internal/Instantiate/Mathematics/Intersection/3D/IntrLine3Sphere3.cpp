#include <Mathematics/IntrLine3Sphere3.h>

namespace gte
{
    template class TIQuery<float, Line3<float>, Sphere3<float>>;
    template class FIQuery<float, Line3<float>, Sphere3<float>>;

    template class TIQuery<double, Line3<double>, Sphere3<double>>;
    template class FIQuery<double, Line3<double>, Sphere3<double>>;
}
