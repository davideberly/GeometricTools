#include <Mathematics/IntrRay2Segment2.h>

namespace gte
{
    template class TIQuery<float, Ray2<float>, Segment2<float>>;
    template class FIQuery<float, Ray2<float>, Segment2<float>>;

    template class TIQuery<double, Ray2<double>, Segment2<double>>;
    template class FIQuery<double, Ray2<double>, Segment2<double>>;
}
