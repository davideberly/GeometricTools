#include <Mathematics/IntrSegment3OrientedBox3.h>

namespace gte
{
    template class TIQuery<float, Segment3<float>, OrientedBox3<float>>;
    template class FIQuery<float, Segment3<float>, OrientedBox3<float>>;

    template class TIQuery<double, Segment3<double>, OrientedBox3<double>>;
    template class FIQuery<double, Segment3<double>, OrientedBox3<double>>;
}
