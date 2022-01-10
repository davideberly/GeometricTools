#include <Mathematics/IntrSegment2OrientedBox2.h>

namespace gte
{
    template class TIQuery<float, Segment2<float>, OrientedBox2<float>>;
    template class FIQuery<float, Segment2<float>, OrientedBox2<float>>;

    template class TIQuery<double, Segment2<double>, OrientedBox2<double>>;
    template class FIQuery<double, Segment2<double>, OrientedBox2<double>>;
}
