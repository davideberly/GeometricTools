#include <Mathematics/IntrLine3Capsule3.h>

namespace gte
{
    template class TIQuery<float, Line3<float>, Capsule3<float>>;
    template class FIQuery<float, Line3<float>, Capsule3<float>>;

    template class TIQuery<double, Line3<double>, Capsule3<double>>;
    template class FIQuery<double, Line3<double>, Capsule3<double>>;
}
