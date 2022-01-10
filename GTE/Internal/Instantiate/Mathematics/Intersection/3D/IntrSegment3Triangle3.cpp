#include <Mathematics/IntrSegment3Triangle3.h>

namespace gte
{
    template class TIQuery<float, Segment3<float>, Triangle3<float>>;
    template class FIQuery<float, Segment3<float>, Triangle3<float>>;

    template class TIQuery<double, Segment3<double>, Triangle3<double>>;
    template class FIQuery<double, Segment3<double>, Triangle3<double>>;
}
