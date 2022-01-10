#include <Mathematics/IntrHalfspace3Triangle3.h>

namespace gte
{
    template class TIQuery<float, Halfspace3<float>, Triangle3<float>>;
    template class FIQuery<float, Halfspace3<float>, Triangle3<float>>;

    template class TIQuery<double, Halfspace3<double>, Triangle3<double>>;
    template class FIQuery<double, Halfspace3<double>, Triangle3<double>>;
}
