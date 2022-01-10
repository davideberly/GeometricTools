#include <Mathematics/IntrSegment3Sphere3.h>

namespace gte
{
    template class TIQuery<float, Segment3<float>, Sphere3<float>>;
    template class FIQuery<float, Segment3<float>, Sphere3<float>>;

    template class TIQuery<double, Segment3<double>, Sphere3<double>>;
    template class FIQuery<double, Segment3<double>, Sphere3<double>>;
}
