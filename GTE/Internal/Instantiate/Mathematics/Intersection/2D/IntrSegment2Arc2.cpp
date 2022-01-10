#include <Mathematics/IntrSegment2Arc2.h>

namespace gte
{
    template class TIQuery<float, Segment2<float>, Arc2<float>>;
    template class FIQuery<float, Segment2<float>, Arc2<float>>;

    template class TIQuery<double, Segment2<double>, Arc2<double>>;
    template class FIQuery<double, Segment2<double>, Arc2<double>>;
}
