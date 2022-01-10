#include <Mathematics/DistRaySegment.h>

namespace gte
{
    template class DCPQuery<float, Ray2<float>, Segment2<float>>;
    template class DCPQuery<double, Ray3<double>, Segment3<double>>;
}
