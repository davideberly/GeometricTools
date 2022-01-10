#include <Mathematics/DistPointSegment.h>

namespace gte
{
    template class DCPQuery<float, Vector<2, float>, Segment<2, float>>;
    template class DCPQuery<double, Vector<3, double>, Segment<3, double>>;
}
