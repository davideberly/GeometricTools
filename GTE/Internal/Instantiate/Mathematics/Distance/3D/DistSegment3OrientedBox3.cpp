#include <Mathematics/DistSegment3OrientedBox3.h>

namespace gte
{
    template class DCPQuery<float, Segment3<float>, OrientedBox3<float>>;
    template class DCPQuery<double, Segment3<double>, OrientedBox3<double>>;
}
