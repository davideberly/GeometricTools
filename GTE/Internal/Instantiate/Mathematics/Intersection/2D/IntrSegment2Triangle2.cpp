#include <Mathematics/IntrSegment2Triangle2.h>

namespace gte
{
    template class TIQuery<float, Segment2<float>, Triangle2<float>>;
    template class FIQuery<float, Segment2<float>, Triangle2<float>>;

    template class TIQuery<double, Segment2<double>, Triangle2<double>>;
    template class FIQuery<double, Segment2<double>, Triangle2<double>>;
}
