#include <Mathematics/IntrSegment2Segment2.h>

namespace gte
{
    template class TIQuery<float, Segment2<float>, Segment2<float>>;
    template class FIQuery<float, Segment2<float>, Segment2<float>>;

    template class TIQuery<double, Segment2<double>, Segment2<double>>;
    template class FIQuery<double, Segment2<double>, Segment2<double>>;
}
