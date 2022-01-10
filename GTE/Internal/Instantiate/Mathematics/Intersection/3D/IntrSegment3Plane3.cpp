#include <Mathematics/IntrSegment3Plane3.h>

namespace gte
{
    template class TIQuery<float, Line3<float>, Plane3<float>>;
    template class FIQuery<float, Line3<float>, Plane3<float>>;

    template class TIQuery<double, Line3<double>, Plane3<double>>;
    template class FIQuery<double, Line3<double>, Plane3<double>>;
}
