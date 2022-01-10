#include <Mathematics/IntrSegment3Capsule3.h>

namespace gte
{
    template class TIQuery<float, Segment3<float>, Capsule3<float>>;
    template class FIQuery<float, Segment3<float>, Capsule3<float>>;

    template class TIQuery<double, Segment3<double>, Capsule3<double>>;
    template class FIQuery<double, Segment3<double>, Capsule3<double>>;
}
