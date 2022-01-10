#include <Mathematics/IntrLine3Triangle3.h>

namespace gte
{
    template class TIQuery<float, Line3<float>, Triangle3<float>>;
    template class FIQuery<float, Line3<float>, Triangle3<float>>;

    template class TIQuery<double, Line3<double>, Triangle3<double>>;
    template class FIQuery<double, Line3<double>, Triangle3<double>>;
}
