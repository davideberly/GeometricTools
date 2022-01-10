#include <Mathematics/IntrLine2Segment2.h>

namespace gte
{
    template class TIQuery<float, Line2<float>, Segment2<float>>;
    template class FIQuery<float, Line2<float>, Segment2<float>>;

    template class TIQuery<double, Line2<double>, Segment2<double>>;
    template class FIQuery<double, Line2<double>, Segment2<double>>;
}
