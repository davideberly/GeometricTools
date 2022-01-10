#include <Mathematics/IntrSegment2Circle2.h>

namespace gte
{
    template class TIQuery<float, Segment2<float>, Circle2<float>>;
    template class FIQuery<float, Segment2<float>, Circle2<float>>;

    template class TIQuery<double, Segment2<double>, Circle2<double>>;
    template class FIQuery<double, Segment2<double>, Circle2<double>>;
}
