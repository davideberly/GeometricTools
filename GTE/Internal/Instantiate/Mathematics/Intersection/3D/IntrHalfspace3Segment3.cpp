#include <Mathematics/IntrHalfspace3Segment3.h>

namespace gte
{
    template class TIQuery<float, Halfspace3<float>, Segment3<float>>;
    template class FIQuery<float, Halfspace3<float>, Segment3<float>>;

    template class TIQuery<double, Halfspace3<double>, Segment3<double>>;
    template class FIQuery<double, Halfspace3<double>, Segment3<double>>;
}
