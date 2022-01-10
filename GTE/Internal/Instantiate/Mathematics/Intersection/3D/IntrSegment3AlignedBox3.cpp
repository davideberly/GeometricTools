#include <Mathematics/IntrSegment3AlignedBox3.h>

namespace gte
{
    template class TIQuery<float, Segment3<float>, AlignedBox3<float>>;
    template class FIQuery<float, Segment3<float>, AlignedBox3<float>>;

    template class TIQuery<double, Segment3<double>, AlignedBox3<double>>;
    template class FIQuery<double, Segment3<double>, AlignedBox3<double>>;
}
