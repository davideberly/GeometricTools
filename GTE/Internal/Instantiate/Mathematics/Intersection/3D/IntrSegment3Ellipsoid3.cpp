#include <Mathematics/IntrSegment3Ellipsoid3.h>

namespace gte
{
    template class TIQuery<float, Segment3<float>, Ellipsoid3<float>>;
    template class FIQuery<float, Segment3<float>, Ellipsoid3<float>>;

    template class TIQuery<double, Segment3<double>, Ellipsoid3<double>>;
    template class FIQuery<double, Segment3<double>, Ellipsoid3<double>>;
}
